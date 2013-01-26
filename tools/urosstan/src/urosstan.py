#!/usr/bin/env python

# Copyright (c) 2012-2013, Politecnico di Milano. All rights reserved.
# 
# Andrea Zoppi <texzk@email.it>
# Martino Migliavacca <martino.migliavacca@gmail.com>
# 
# http://airlab.elet.polimi.it/
# http://www.openrobots.com/
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys
import os
import re

# GKD regex
funcnameregex_simple = re.compile(';; Function \((\S+)\)\s*')
funcnameregex_cplx = re.compile(';; Function (.*) \((\S+)(,.*)?\)\s*')
callregex = re.compile('.*\(call.*"(.*)".*');
refregex = re.compile('.*\(symbol_ref.*"(.*)".*')

# SU regex
stackregex  = re.compile('\S+:\S+:\S+:(.*)\s+(\S+)\s+(\S+)\s*')

# NM regex
staticregex = re.compile('[0-9a-fA-F]*\s+t\s+(\S+)\s*')

#=============================================================================#

def unquote(string):
    string = string.strip()
    if len(string) >= 2 and \
       ((string[0] == '"' and string[-1] == '"') or \
        (string[0] == "'" and string[-1] == "'")):
        string = string[1:-1]
    return string

#=============================================================================#

class Function:
    def __init__(self, sourceUnit, name, unmangled, stackUsage, isStatic):
        self.sourceUnit = sourceUnit
        self.name = name
        self.unmangled = unmangled
        self.stackUsage = stackUsage
        self.isStatic = isStatic
        if self.sourceUnit:
            self.__hash = hash(self.sourceUnit.sourcePath + ':' + self.name)
        else:
            self.__hash = hash(':' + self.name)
        
        self.unresolved = set()     # { "callee", ... }
        self.resolved = set()       # { Function() callee, ... }
        self.refs = set()           # { "ref", ... }
        
    def __hash__(self):
        return self.__hash
        
    def addUnresolved(self, calleeName):
        self.unresolved.add(calleeName)
        
    def addRef(self, refName):
        self.refs.add(refName)
    
    def fixResolved(self, calleeFunc):
        if calleeFunc.name in self.unresolved:
            self.unresolved.remove(calleeFunc.name)
            self.resolved.add(calleeFunc)
            
    def maxUsage(self, callpath):
        if self in callpath:
            return (self.stackUsage[0], callpath + [self])
        if len(self.resolved) == 0:
            return (self.stackUsage[0], callpath + [self, None])
        
        maxsubstk = -1
        maxpath = None
        for callee in self.resolved:
            (curstk, curpath) = callee.maxUsage(callpath + [self])
            if maxsubstk < curstk:
                maxsubstk = curstk
                maxpath = curpath
        
        return (self.stackUsage[0] + maxsubstk, maxpath)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

class SourceUnit:
    def __init__(self, sourcePath, gkdPath, suPath, nmPath):
        self.sourcePath = sourcePath
        self.gkdPath = gkdPath
        self.suPath = suPath
        self.nmPath = nmPath
        
        self._statics = set()   # { "static", ... }
        self._stacks = {}       # { "caller" : #usage, ... }
        self.funcs = set()      # { Function(), ... }
    
    def elaborate(self):
        print 'Elaborating unit: %s' % self.sourcePath
        
        # Load information from build files, and create the function set
        self._load_su(self.suPath)
        self._load_nm(self.nmPath)
        self._load_gkd(self.gkdPath)
        
        # Resolve static calls
        staticnames = [f.name for f in self.funcs if f.isStatic]
        for func in self.funcs:
            for resolved in [f for f in self.funcs if f.name in staticnames]:
                func.fixResolved(resolved)

    def _load_gkd(self, path):
        self.funcs = set()
        lines = []
        curfunc = None
        if os.path.exists(path):
            with open(path, 'r') as f:
                lines = f.readlines()
        else:
            raise ValueError('Invalid GKD file path: %s' % path)
        
        for line in lines:
            # Look for function definitions
            m = funcnameregex_simple.match(line)
            if m:
                unmangled = funcname = m.group(1)
            else:
                m = funcnameregex_cplx.match(line)
                if m:
                    unmangled = m.group(1)
                    funcname = m.group(2)
            if m:
                # Add the function to the function set
                if funcname in self._stacks:
                    usage = self._stacks[funcname]
                else:
                    usage = (0, 'static')
                    print 'No stack usage for: %s' % funcname
                static = funcname in self._statics
                curfunc = Function(self, funcname, unmangled, usage, static)
                self.funcs.add(curfunc)
                continue
            
            # Look for direct calls
            m = callregex.match(line)
            if m:
                # Unresolved call found inside the current function
                callee = m.group(1)
                curfunc.addUnresolved(callee)
                continue
            
            # Look for references
            m = refregex.match(line)
            if m:
                # Generic reference found inside the current function, may turn
                # out it is a function later
                ref = m.group(1)
                curfunc.addRef(ref)
                continue

    def _load_su(self, path):
        self._stacks = {}
        lines = []
        if os.path.exists(path):
            with open(path, 'r') as f:
                lines = f.readlines()
        else:
            raise ValueError('Invalid SU file path: %s' % path)
        
        for line in lines:
            m = stackregex.match(line)
            if m:
                caller = m.group(1)
                usage = m.group(2)
                mode = m.group(3)
                assert mode in ['static', 'dynamic,bounded', 'dynamic']
                self._stacks[caller] = (int(usage), mode)
    
    def _load_nm(self, path):
        self._statics = set()
        lines = []
        if os.path.exists(path):
            with open(path, 'r') as f:
                lines = f.readlines()
        else:
            raise ValueError('Invalid NM file path: %s' % path)
        
        for line in lines:
            m = staticregex.match(line)
            if m:
                staticfunc = m.group(1)
                self._statics.add(staticfunc)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

class StaticAnalysis:
    def __init__(self, cfgPath):
        self.cfgPath = cfgPath
        self.cfiles = set()
        self.cppfiles = set()
        
        self.cfgDir = os.sep.join(os.path.split(self.cfgPath)[:-1])
        if len(self.cfgDir) == 0: self.cfgDir = '.'
        
        self.units = []         # [ SourceUnit("*.c[pp]", "*.gkd", "*.su", "*.nm"), ... ]
        self.opts =  {          # { "key" : "value", ... }
            'outDir'            : '.',
            'makefileDir'       : '.',
        }
        self.terms = {}         # { "terminator" : #usage, ... }
        self.entrypts = {}      # { "entry_point": #usage_bias, ... }
        self.funcs = set()      # { Function(), ... }
        
        self._load_cfg(cfgPath)
        
    def _load_cfg(self, path):
        if path == '.':
            lines = sys.stdin.readlines()
        else:
            if os.path.exists(path):
                with open(path, 'r') as f:
                    lines = f.readlines()
            else:
                raise ValueError('Invalid CFG file path: %s' % path)
        
        modes = [ '[options]', '[terminators]', '[entrypoints]', '[sourceunits]' ]
        mode = None
        modeidx = -1
        for line in lines:
            line = (line.partition('#'))[0].strip()
            if len(line) == 0: continue
            if line.lower() in modes:
                mode = line.lower()
                if modes.index(mode) != modeidx + 1:
                    text = 'Sections must be in the order:\n'
                    for m in modes: text += '  %s\n' % m
                    raise ValueError(text)
                modeidx += 1
                if modeidx == 3:
                    # Fix makefile path
                    mkdir = self.cfgDir + os.sep + self.opts['makefileDir']
                    self.opts['makefileDir'] = os.path.normpath(mkdir)
                continue
            
            if not '=' in line:
                raise ValueError("Cannot find '=' in line: %s" % line)
            eqidx = line.index('=')
            key = unquote(line[:eqidx])
            value = line[eqidx+1:].strip()
            
            if mode == '[options]':
                value = unquote(value)
                if not self.opts.has_key(key):
                    raise ValueError('Invalid option: ' + key)
                self.opts[key] = value
            
            elif mode == '[terminators]':
                self.terms[key] = int(unquote(value))
                
            elif mode == '[entrypoints]':
                self.entrypts[key] = int(unquote(value))
                
            elif mode == '[sourceunits]':
                objs = value.split('|')
                if len(objs) != 3:
                    raise ValueError('Expecting 3 object files, got ' + len(objs))
                
                key = os.path.normpath(self.opts['makefileDir'] + os.sep + unquote(key))
                gkd = os.path.normpath(self.opts['makefileDir'] + os.sep + unquote(objs[0]))
                su  = os.path.normpath(self.opts['makefileDir'] + os.sep + unquote(objs[1]))
                nm  = os.path.normpath(self.opts['makefileDir'] + os.sep + unquote(objs[2]))
                
                unit = SourceUnit(key, gkd, su, nm)
                self.units.append(unit)
                
        if modeidx != len(modes) - 1:
            raise ValueError('Not all the sections were defined')
        
    def elaborate(self):
        self.funcs = set()
        
        # Elaborate source units
        print '### Unit elaboration ###'
        for unit in self.units:
            unit.elaborate()
        print ''
        
        # Populate the function set
        for unit in self.units:
            self.funcs = self.funcs.union(unit.funcs)
        
        # Resolve globally declared terminators
        for termname in self.terms:
            found = False
            for func in self.funcs:
                if func.name == termname and not func.isStatic:
                    func.stackUsage = (self.terms[func.name], 'static')
                    func.resolved = set()
                    func.unresolved = set()
                    found = True
            if not found:
                dummyfunc = Function(None, termname, termname, self.terms[termname], False)
                self.funcs.add(dummyfunc)
        
        # Resolve globally declared functions
        entryfuncs = []
        somenotfound = False
        for name in self.entrypts:
            found = False
            for func in self.funcs:
                if func.name == name:
                    entryfuncs.append(func)
                    found = True
            if not found:
                print 'Cannot find entry point: %s' % name
                somenotfound = True
        if somenotfound: print ''
        entryfuncs = sorted(entryfuncs, cmp = lambda x, y: cmp(x.name, y.name))
        toresolve = {}
        for func in self.funcs:
            for unresname in func.unresolved:
                if not toresolve.has_key(unresname):
                    toresolve[unresname] = [func]
                else:
                    toresolve[unresname].append(func)
        for func in [f for f in self.funcs if func.name in toresolve]:
            if toresolve.has_key(func.name):
                for caller in toresolve[func.name]:
                    caller.fixResolved(func)
        
        unresolved = set()
        for func in self.funcs:
            unresolved = unresolved.union(func.unresolved)
        print '### Unresolved symbols ###'
        for name in sorted(unresolved):
            print '  ' + name
        print ''
        
        # Compute the maximum usage for each starting function
        totals = {}
        for entry in entryfuncs:
            print '### Entry point function: %s ###' % entry.name
            
            basestk = total = self.entrypts[entry.name]
            maxpath = entry.maxUsage([])[1]
            
            maxnamelen = -1
            maxdeltalen = len(str(basestk))
            for callee in maxpath:
                if callee == None: break
                if maxnamelen < len(callee.name):
                    maxnamelen = len(callee.name)
                if maxdeltalen < len(str(callee.stackUsage[0])):
                    maxdeltalen = len(str(callee.stackUsage[0]))
                total += callee.stackUsage[0]
            maxtotallen = len(str(total))
            totals[entry] = total
            
            namespaces = ' ' * (maxnamelen - len('(entry point)'))
            deltaspaces = ' ' * (maxdeltalen - len(str(basestk)))
            totalspaces = ' ' * (maxtotallen - len(str(basestk)))
            print 'Max stack depth = %d bytes:' % total
            print '  (entry point)%s = %s%d (%s%d)' % (namespaces,
                                                       deltaspaces, basestk,
                                                       totalspaces, basestk)
            
            total = self.entrypts[entry.name]
            for callee in maxpath:
                if callee == None: break
                delta = callee.stackUsage[0]
                total += delta
                namespaces = ' ' * (maxnamelen - len(callee.name))
                deltaspaces = ' ' * (maxdeltalen - len(str(callee.stackUsage[0])))
                totalspaces = ' ' * (maxtotallen - len(str(total)))
                print '  %s%s = %s%d (%s%d)' % (callee.name, namespaces,
                                                deltaspaces, delta,
                                                totalspaces, total)
            if callee != None:
                print '  recursion to: %s' % callee.name        
            print ''
        
        print '### Summary: ###'
        maxtotallen = -1
        maxnamelen = -1
        for entry in entryfuncs:
            if maxtotallen < len(str(totals[entry])):
                maxtotallen = len(str(totals[entry]))
            if maxnamelen < len(entry.name):
                maxnamelen = len(entry.name)
        for entry in entryfuncs:
            namespaces = ' ' * (maxnamelen - len(entry.name))
            totalspaces = ' ' * (maxtotallen - len(str(totals[entry])))
            print '%s%s = %s%d' % (entry.name, namespaces, totalspaces, totals[entry])

###############################################################################

def print_usage():
    print 'Usage:\t%s <config_file>\n' % os.path.basename(sys.argv[0])
    print 'If <config_file> is ".", it is read from the standard input.'
    sys.stdout.flush()

def main():
    if len(sys.argv) != 2:
        print_usage()
        exit()
    
    cfgPath = sys.argv[1]
    sa = StaticAnalysis(cfgPath)
    sa.elaborate()

main()
