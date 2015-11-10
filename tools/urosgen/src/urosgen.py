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

import os
import sys
import subprocess
import re
from locale import str

primitive_map = {
  'bool'        : 'uint8_t',
  'int8'        : 'int8_t',
  'uint8'       : 'uint8_t',
  'int16'       : 'int16_t',
  'uint16'      : 'uint16_t',
  'int32'       : 'int32_t',
  'uint32'      : 'uint32_t',
  'int64'       : 'int64_t',
  'uint64'      : 'uint64_t',
  'float32'     : 'float',
  'float64'     : 'double',
  
  'string'      : 'UrosString',
  'time'        : 'uros_time_t',
  'duration'    : 'uros_time_t',
  
  'char'        : 'int8_t',
  'byte'        : 'uint8_t',
  'int'         : 'int32_t',
  'uint'        : 'uint32_t'
}

tab = '  '
ROSMSG = 'rosmsg'
ROSSRV = 'rossrv'

nameregex = re.compile('[a-zA-Z][a-zA-Z0-9_]*')
pathregex = re.compile('[a-zA-Z][a-zA-Z0-9_]*([/][a-zA-Z][a-zA-Z0-9_]*)*')
fixarrayregex = re.compile('^(.*)\[([0-9]*)\]$')

#=============================================================================#

def str2bool(s):
    s = s.strip().lower()
    if s.isdigit():
        return int(s) != 0
    else:
        return s in [ 'true', 't', 'y', 'yes', 'on', 'enable', 'enabled' ]

def valid_path(path):
    return pathregex.match(path) != None

def valid_name(name):
    return nameregex.match(name) != None

def mangled_name(rostype):
    return rostype.replace('/', '__')

def next_tab(curpos, tabSize=4):
    return curpos + tabSize - curpos % tabSize

def addslashes(text):
    def replace(c):
        return {
            "\\" : "\\\\",
            "\"" : "\\\"",
            "\'" : "\\\'",
            "\n" : "\\n",
            "\r" : "\\r",
            "\t" : "\\t",
            "\v" : "\\v",
        }.get(c, c)
    escaped = ""
    for c in text:
        escaped += replace(c)
    return escaped

def banner_big(title):
    text = ""
    if len(title) <= 73:
        text += '/*=' + ('=' * 73) + '=*/\n'
        text += '/* ' + title + (' ' * (73 - len(title))) + ' */\n'
        text += '/*=' + ('=' * 73) + '=*/'
    else:
        text += '/*=' + ('=' * len(title)) + '=*/\n'
        text += '/* ' + title + ' */\n'
        text += '/=*' + ('=' * len(title)) + '*=/'
    return text
    
def banner_small(title):
    if len(title) <= 67:
        return '/*~~~ ' + title + ' ' + ('~' * (67 - len(title))) + '~~~*/'
    else:
        return '/*~~~ %s ~~~*/' % title

def sorted_deps(deps):
    deps = list(deps)
    pending = []
    for e in deps:
        if e[0] not in pending: pending.append(e[0])
        if e[1] not in pending: pending.append(e[1])
    
    sortednodes = []
    while len(pending) > 0:
        fanout = [ 0 ] * len(pending)
        for i in range(len(pending)):
            for e in deps:
                if e[0] == pending[i]:
                    fanout[i] += 1
        for i in reversed(range(len(pending))):
            if fanout[i] == 0:
                sortednodes.append(pending[i])
                for j in reversed(range(len(deps))):
                    if deps[j][1] == pending[i]:
                        del deps[j]
                del pending[i]
    return sortednodes

#=============================================================================#

class Field:
    def __init__(self, rostype, ctype, name, arraylen=0):
        self.rostype = rostype
        self.name = name
        self.arraylen = arraylen
        self.ctype = ctype
        self.cname = mangled_name(name)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

class Const:
    def __init__(self, rostype, ctype, name, value):
        self.rostype = rostype
        self.name = name
        self.value = value
        self.ctype = ctype
        self.cname = mangled_name(name)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

class MsgType:
    def __init__(self, name, _autoload=True):
        self.name = name
        self.cname = "msg__" + mangled_name(name)
        self.consts = []
        self.fields = []
        self.cplxtypes = {}
        self.md5str = None
        self.ctype = "struct " + self.cname
        self.desc = ""
        self.uses_vartypes = False
        self.uses_arrays = False
        
        if _autoload:
            self._load()
    
    def get_complextypes(self):
        cplxtypes = self.cplxtypes.copy()
        cplxtypes[self.name] = self
        for k1 in self.cplxtypes:
            subtypes = self.cplxtypes[k1].get_complextypes()
            for k2 in subtypes:
                if not k2 in cplxtypes:
                    cplxtypes[k2] = subtypes[k2]
        return cplxtypes
    
    def get_deps(self):
        deps = []
        for k in self.cplxtypes:
            edge = (self.name, k)
            deps.append(edge)
            for sd in self.cplxtypes[k].get_deps():
                if not sd in deps:
                    deps.append(sd)
        return deps
    
    def _load(self):
        if not '/' in self.name:
            self.name = 'std_msgs/' + self.name
        
        md5str = subprocess.check_output([ROSMSG, 'md5', self.name])
        self.md5str = md5str.strip()
        
        lines = subprocess.check_output([ROSMSG, 'show', self.name])
        lines = lines.split('\n') + [""]
        startline = 0
        endline = len(lines) - 1
        self._process_subscript(lines, startline, endline)
        
        self.uses_vartypes = False
        for f in self.fields:
            if f.rostype == 'string' or f.arraylen == '*':
                self.uses_vartypes = True
                break
        for k in self.cplxtypes:
            if self.cplxtypes[k].uses_vartypes:
                self.uses_vartypes = True
                break
            
        self.uses_arrays = False
        for f in self.fields:
            if f.arraylen and (f.rostype == 'string' or not primitive_map.has_key(f.rostype)):
                self.uses_arrays = True
                break
    
    def _process_subscript(self, lines, startline, endline):
        i = startline
        while i <= endline:
            # Resolve tokens
            line = lines[i]
            stripped = line.strip()
            if len(stripped) == 0:
                i += 1
                continue
            tokens = stripped.split()
            assert len(tokens) == 2
            rostype = tokens[0]
            assert valid_path(rostype)
            name = tokens[1]
            assert valid_name(name)
            
            match = fixarrayregex.match(rostype)
            if match:
                rostype = match.group(1)
                arraylen = match.group(2)
                arraylen = int(arraylen) if len(arraylen) > 0 else '*'
            else:
                arraylen = 0
            
            if not primitive_map.has_key(rostype):
                if not '/' in rostype:
                    rostype = 'std_msgs/' + rostype
                substart = i + 1
                subend = endline;
                startlead = len(lines[substart]) - len(lines[substart].lstrip())
                for j in range(substart, endline + 1):
                    endlead = len(lines[j]) - len(lines[j].lstrip())
                    if endlead < startlead:
                        subend = j
                        break
                assert subend > substart
                
                if self.cplxtypes.has_key(rostype):
                    # Complex type already known
                    cplxtype = self.cplxtypes[rostype]
                else:
                    # Unknown complex type, dig it
                    cplxtype = MsgType(rostype)
                    self.cplxtypes[rostype] = cplxtype
                
                i = subend
            else:
                i += 1
            
            if primitive_map.has_key(rostype):
                ctype = primitive_map[rostype]
            else:
                ctype = self.cplxtypes[rostype].ctype
            if '=' in name:
                tokens = name.split('=')
                name = tokens[0]
                value = tokens[1]
                const = Const(rostype, ctype, name, value)
                self.consts.append(const)
            else:
                field = Field(rostype, ctype, name, arraylen)
                self.fields.append(field)
    
    def gen_struct_body(self, maxtype, maxname, comments=True):
        text = ""
        for f in self.fields:
            if comments:
                text += '\n' + tab + '/** @brief TODO: @p %s description.*/\n' % f.name
            if f.arraylen == '*':
                line = tab + 'UROS_VARARR(%s)' % f.ctype
            else:
                line = tab + f.ctype
            line += ' ' * (maxtype - len(line)) + f.cname
            if f.arraylen and f.arraylen != '*':
                line += '[%d]' % f.arraylen
            text += line + ';\n'
        return text
    
    def gen_struct(self, comments=True):
        maxtype = 0
        maxname = 0
        for f in self.fields:
            if f.arraylen == '*':
                typelen = len('UROS_VARARR(%s)' % f.ctype)
            else:
                typelen = len(f.ctype)
            if maxtype < typelen:
                maxtype = typelen
            
            if f.arraylen and f.arraylen != '*':
                namelen = len('%s[%d]' % (f.cname, f.arraylen));
            else:
                namelen = len(f.cname)
            if maxname < namelen:
                maxname = namelen
        
        maxtype = next_tab(len(tab) + maxtype)
        text = '/**\n'
        text += ' * @brief   TCPROS <tt>%s</tt> message descriptor.\n' % self.name
        text += ' * @details MD5 sum: <tt>%s</tt>.\n' % self.md5str
        text += ' */\n'
        text += '%s {\n' % self.ctype
        if len(self.fields) > 0:
            text += self.gen_struct_body(maxtype, maxname, comments)
        else:
            text += tab + '/* This message type has no fields.*/\n'
            text += tab + 'uint8_t _dummy;\n'
        text += '};'
        return text
    
    def gen_const_decls(self, comments=True):
        text = ""
        strings = []
        ints = []
        maxlen = 0
        for c in self.consts:
            if c.rostype == 'string':
                strings.append(c)
            else:
                ints.append(c)
                namelen = 8 + len(self.cname) + 2 + len(c.cname)
                namelen = next_tab(namelen)
            if maxlen < namelen: maxlen = namelen
        
        for c in strings:
            cname = '%s__%s' % (self.cname, c.cname)
            valstr = addslashes(c.value)
            if comments:
                text += '/** @see @p %s */\n' % cname
            text += '#define %s__SZ \\\n' % cname
            text += tab + '"%s"\n' % valstr
            text += 'extern const UrosString %s;\n\n' % cname
            
        for c in ints:
            cname = '%s__%s' % (self.cname, c.cname)
            if comments:
                if c != ints[0]: text += '\n'
                text += '/** @brief TODO: <tt>%s.%s</tt> description.*/\n' % (self.name, c.name)
            line = '#define %s ' % cname
            line += ' ' * (maxlen - len(line))
            line += ' ' * (next_tab(len(line)) - len(line))
            text += line + '((%s)%s)\n' % (c.ctype, c.value)
        
        if len(text) > 0:
            text += '\n'
        return text
    
    def gen_const_defs(self, comments=True):
        text = ""
        for c in self.consts:
            if c.rostype == 'string':
                cname = '%s__%s' % (self.cname, c.cname)
                valstr = addslashes(c.value)
                if comments:
                    text += '/** @brief TODO: <tt>%s.%s</tt> description.*/\n' % (self.name, c.name)
                text += 'const UrosString %s = \n' % cname
                text += tab + '{ %d, "%s" };\n\n' % (len(valstr), valstr)
        return text
    
    def gen_length_sig(self):
        text = 'size_t length_%s(\n' % self.cname
        text += tab + '%s *objp\n' % self.ctype
        text += ')'
        return text
    
    def gen_length_body(self):
        text = ""
        for f in self.fields:
            if f.arraylen:
                if f.arraylen == '*':
                    lenstr = 'objp->%s.length' % f.cname
                    enpstr = 'objp->%s.entriesp' % f.cname
                    text += tab + 'length += sizeof(uint32_t);\n'
                else:
                    lenstr = str(f.arraylen)
                    enpstr = f.name
                
                if f.rostype == 'string':
                    text += tab + 'length += (size_t)%s * sizeof(uint32_t);\n' % lenstr
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += tab*2 + 'length += %s[i].length;\n' % enpstr
                    text += tab + '}\n'
                elif primitive_map.has_key(f.rostype):
                    text += tab + 'length += (size_t)%s * sizeof(%s);\n' % (lenstr, f.ctype)
                else:
                    cplxtype = self.cplxtypes[f.rostype]
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += tab*2 + 'length += length_%s(&%s[i]);\n' % (cplxtype.cname, enpstr)
                    text += tab + '}\n'
            
            elif f.rostype == 'string':
                text += tab + 'length += sizeof(uint32_t) + objp->%s.length;\n' % f.cname
            elif primitive_map.has_key(f.rostype):
                text += tab + 'length += sizeof(%s);\n' % f.ctype
            else:
                cplxtype = self.cplxtypes[f.rostype]
                text += tab + 'length += length_%s(&objp->%s);\n' % (cplxtype.cname, f.cname)
        
        return text
    
    def gen_length(self):
        text = '/**\n'
        text += ' * @brief   Content length of a TCPROS <tt>%s</tt> message.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Length of the TCPROS message contents, in bytes.\n'
        text += ' */\n'
        text += self.gen_length_sig() + ' {\n'
        text += tab + 'size_t length = 0;\n'
        text += (tab + 'uint32_t i;\n\n') if self.uses_arrays else '\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.gen_length_body()
        if len(body) > 0:
            text += body + '\n'
            objpUsed = False
            for f in self.fields:
                if f.rostype == 'string' or not primitive_map.has_key(f.rostype) or f.arraylen == '*':
                    objpUsed = True
                    break
            if not objpUsed:
                text += tab + '(void)objp;\n'
        else:
            text += tab + '/* Nothing to measure.*/\n'
            text += tab + '(void)objp;\n'
        text += tab + 'return length;\n'
        text += '}'
        return text
    
    def gen_init_sig(self):
        text = 'void init_%s(\n' % self.cname
        text += tab + '%s *objp\n' % self.ctype
        text += ')'
        return text
    
    def gen_init_body(self):
        text = ""
        for f in self.fields:
            if f.arraylen:
                if f.arraylen == '*':
                    lenstr = 'objp->%s.length' % f.cname
                    enpstr = 'objp->%s.entriesp' % f.cname
                    text += tab + 'urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->%s);\n' % f.cname
                else:
                    lenstr = str(f.arraylen)
                    enpstr = f.name
                
                looptext = ""                
                if f.rostype == 'string':
                    looptext += tab*2 + 'urosStringObjectInit(&%s[i]);\n' % enpstr
                elif not primitive_map.has_key(f.rostype):
                    cplxtype = self.cplxtypes[f.rostype]
                    looptext += tab*2 + 'init_%s(&%s[i]);\n' % (cplxtype.cname, enpstr)
                if len(looptext) > 0:
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += looptext
                    text += tab + '}\n'
                
            elif f.rostype == 'string':
                text += tab + 'urosStringObjectInit(&objp->%s);\n' % f.cname
            elif not primitive_map.has_key(f.rostype):
                cplxtype = self.cplxtypes[f.rostype]
                text += tab + 'init_%s(&objp->%s);\n' % (cplxtype.cname, f.cname)
        return text
    
    def gen_init(self):
        uses_arrays = False
        for f in self.fields:
            if f.arraylen and (f.rostype == 'string' or not primitive_map.has_key(f.rostype)):
                uses_arrays = True
                break
        
        text = '/**\n'
        text += ' * @brief   Initializes a TCPROS <tt>%s</tt> message.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an allocated <code>%s</code> object.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_init_sig() + ' {\n'
        if uses_arrays:
            text += tab + 'uint32_t i;\n\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.gen_init_body()
        if len(body) > 0:
            text += body
        else:
            text += tab + '/* Nothing to initialize.*/\n'
            text += tab + '(void)objp;\n'
        text += '}'
        return text
    
    def gen_clean_sig(self):
        text = 'void clean_%s(\n' % self.cname
        text += tab + '%s *objp\n' % self.ctype
        text += ')'
        return text
    
    def gen_clean_body(self):
        text = ""
        for f in self.fields:
            if f.arraylen:
                if f.arraylen == '*':
                    lenstr = 'objp->%s.length' % f.cname
                    enpstr = 'objp->%s.entriesp' % f.cname
                else:
                    lenstr = str(f.arraylen)
                    enpstr = f.name
                
                looptext = ""                
                if f.rostype == 'string':
                    looptext = tab*2 + 'urosStringClean(&%s[i]);\n' % enpstr
                elif not primitive_map.has_key(f.rostype):
                    cplxtype = self.cplxtypes[f.rostype]
                    looptext = tab*2 + 'clean_%s(&%s[i]);\n' % (cplxtype.cname, enpstr)
                if len(looptext) > 0:
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += looptext
                    text += tab + '}\n'
                
                if f.arraylen == '*':
                    text += tab + 'urosTcpRosArrayClean((UrosTcpRosArray *)&objp->%s);\n' % f.cname
                
            elif f.rostype == 'string':
                text += tab + 'urosStringClean(&objp->%s);\n' % f.cname
            elif not primitive_map.has_key(f.rostype):
                cplxtype = self.cplxtypes[f.rostype]
                text += tab + 'clean_%s(&objp->%s);\n' % (cplxtype.cname, f.cname)
        return text
    
    def gen_clean(self):
        text = '/**\n'
        text += ' * @brief   Cleans a TCPROS <tt>%s</tt> message.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object, or @p NULL.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_clean_sig() + ' {\n'
        if self.uses_arrays:
            text += tab + 'uint32_t i;\n\n'
        body = self.gen_clean_body()
        if len(body) > 0:
            text += tab + 'if (objp == NULL) { return; }\n\n'
            text += body
        else:
            text += tab + '/* Nothing to clean.*/\n'
            text += tab + '(void)objp;\n'
        text += '}'
        return text
    
    def gen_recv_sig(self):
        text = 'uros_err_t recv_%s(\n' % self.cname
        text += tab + 'UrosTcpRosStatus *tcpstp,\n'
        text += tab + '%s *objp\n' % self.ctype
        text += ')'
        return text
    
    def gen_recv_body(self):
        text = ""
        for f in self.fields:
            if f.arraylen:
                if f.arraylen == '*':
                    lenstr = 'objp->%s.length' % f.cname
                    enpstr = 'objp->%s.entriesp' % f.cname
                    text += tab + 'urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->%s);\n' % f.cname
                    text += tab + 'urosTcpRosRecvRaw(tcpstp, %s); _CHKOK\n' % lenstr
                    line = tab + '%s = urosArrayNew(' % enpstr
                    text += line + 'NULL, %s,\n' % lenstr
                    text += ' ' * len(line) + '%s);\n' % f.ctype
                    text += tab + 'if (%s == NULL) { ' % enpstr
                    text += 'tcpstp->err = UROS_ERR_NOMEM; goto _error; }\n'
                else:
                    lenstr = str(f.arraylen)
                    enpstr = 'objp->' + f.name
                
                if f.rostype == 'string':
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += tab*2 + 'urosTcpRosRecvString(tcpstp, &%s[i]); _CHKOK\n' % enpstr
                    text += tab + '}\n'
                elif primitive_map.has_key(f.rostype):
                    text += tab + 'urosTcpRosRecv(tcpstp, %s,\n' % enpstr
                    text += tab + '               (size_t)%s * sizeof(%s)); _CHKOK\n' % (lenstr, f.ctype)
                else:
                    cplxtype = self.cplxtypes[f.rostype]
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += tab*2 + 'recv_%s(tcpstp, &%s[i]); _CHKOK\n' % (cplxtype.cname, enpstr)
                    text += tab + '}\n'
                
            elif f.rostype == "string":
                text += tab + 'urosTcpRosRecvString(tcpstp, &objp->%s); _CHKOK\n' % f.cname
            elif primitive_map.has_key(f.rostype):
                text += tab + 'urosTcpRosRecvRaw(tcpstp, objp->%s); _CHKOK\n' % f.cname
            else:
                cplxtype = self.cplxtypes[f.rostype]
                text += tab + 'recv_%s(tcpstp, &objp->%s); _CHKOK\n' % (cplxtype.cname, f.cname)
        
        return text
    
    def gen_recv(self):
        text = '/**\n'
        text += ' * @brief   Receives a TCPROS <tt>%s</tt> message.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @param[out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_recv_sig() + ' {\n'
        if self.uses_arrays:
            text += tab + 'uint32_t i;\n\n'
        text += tab + 'urosAssert(tcpstp != NULL);\n'
        text += tab + 'urosAssert(urosConnIsValid(tcpstp->csp));\n'
        text += tab + 'urosAssert(objp != NULL);\n'
        body = self.gen_recv_body()
        if len(body) > 0:
            text += '#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }\n\n'
            text += body + '\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
            text += '_error:\n'
            text += tab + 'clean_%s(objp);\n' % self.cname
            text += tab + 'return tcpstp->err;\n'
            text += '#undef _CHKOK\n'
        else:
            text += '\n'
            text += tab + '/* Nothing to receive.*/\n'
            text += tab + '(void)objp;\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
        text += '}'
        return text
    
    def gen_send_sig(self):
        text = 'uros_err_t send_%s(\n' % self.cname
        text += tab + 'UrosTcpRosStatus *tcpstp,\n'
        text += tab + '%s *objp\n' % self.ctype
        text += ')'
        return text
    
    def gen_send_body(self):
        text = ""
        for f in self.fields:
            if f.arraylen:
                if f.arraylen == '*':
                    lenstr = 'objp->%s.length' % f.cname
                    enpstr = 'objp->%s.entriesp' % f.cname
                    text += tab + 'urosTcpRosSendRaw(tcpstp, %s); _CHKOK\n' % lenstr
                else:
                    lenstr = str(f.arraylen)
                    enpstr = 'objp->' + f.name
                
                if f.rostype == 'string':
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += tab*2 + 'urosTcpRosSendString(tcpstp, &%s[i]); _CHKOK\n' % enpstr
                    text += tab + '}\n'
                elif primitive_map.has_key(f.rostype):
                    text += tab + 'urosTcpRosSend(tcpstp, %s,\n' % enpstr
                    text += tab + '               (size_t)%s * sizeof(%s)); _CHKOK\n' % (lenstr, f.ctype)
                else:
                    cplxtype = self.cplxtypes[f.rostype]
                    text += tab + 'for (i = 0; i < %s; ++i) {\n' % lenstr
                    text += tab*2 + 'send_%s(tcpstp, &%s[i]); _CHKOK\n' % (cplxtype.cname, enpstr)
                    text += tab + '}\n'
                
            elif f.rostype == 'string':
                text += tab + 'urosTcpRosSendString(tcpstp, &objp->%s); _CHKOK\n' % f.cname
            elif primitive_map.has_key(f.rostype):
                text += tab + 'urosTcpRosSendRaw(tcpstp, objp->%s); _CHKOK\n' % f.cname
            else:
                cplxtype = self.cplxtypes[f.rostype]
                text += tab + 'send_%s(tcpstp, &objp->%s); _CHKOK\n' % (cplxtype.cname, f.cname)
        
        return text
    
    def gen_send(self):
        text = '/**\n'
        text += ' * @brief   Sends a TCPROS <tt>%s</tt> message.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @param[in] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_send_sig() + ' {\n'
        if self.uses_arrays:
            text += tab + 'uint32_t i;\n\n'
        text += tab + 'urosAssert(tcpstp != NULL);\n'
        text += tab + 'urosAssert(urosConnIsValid(tcpstp->csp));\n'
        text += tab + 'urosAssert(objp != NULL);\n'
        body = self.gen_send_body()
        if len(body) > 0:
            text += '#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }\n\n'
            text += body + '\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
            text += '#undef _CHKOK\n'
        else:
            text += '\n'
            text += tab + '/* Nothing to send.*/\n'
            text += tab + '(void)objp;\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
        text += '}'
        return text

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
        
class SrvType:
    def __init__(self, name, _autoload=True):
        self.name = name
        self.cname = "srv__" + mangled_name(name)
        self.md5str = None
        self.ctype = "struct " + self.cname
        self.intype = None
        self.outtype = None
        
        if _autoload:
            self._load()
    
    def get_complextypes(self):
        cplxtypes = self.intype.get_complextypes()
        del cplxtypes[self.name]
        cplxtypes2 = self.outtype.get_complextypes()
        del cplxtypes2[self.name]
        
        for k in cplxtypes2:
            if not k in cplxtypes:
                cplxtypes[k] = cplxtypes2[k]
        
        return cplxtypes
    
    def get_deps(self, notSelf=False):
        deps = []
        
        for k in self.intype.cplxtypes:
            if not notSelf:
                edge = (self.name, k)
                deps.append(edge)
            for sd in self.intype.cplxtypes[k].get_deps():
                if notSelf and (sd[0] == self.name or sd[1] == self.name):
                    continue
                if not sd in deps:
                    deps.append(sd)
        
        for k in self.outtype.cplxtypes:
            if not notSelf:
                edge = (self.name, k)
                if not edge in deps: deps.append(edge)
            for sd in self.outtype.cplxtypes[k].get_deps():
                if notSelf and (sd[0] == self.name or sd[1] == self.name):
                    continue
                if not sd in deps:
                    deps.append(sd)
        
        return deps
    
    def _load(self):
        md5str = subprocess.check_output([ROSSRV, 'md5', self.name])
        self.md5str = md5str.strip()
        
        lines = subprocess.check_output([ROSSRV, 'show', self.name])
        lines = lines.split('\n') + [""]
        sep = None
        for i in range(len(lines)):
            if lines[i].strip() == '---':
                lines[i] = ""
                sep = i
                break
        assert sep != None
        
        intype = MsgType("", False)
        intype.name = self.name
        intype.cname = 'in_' + self.cname
        intype.ctype = 'struct in_' + self.cname
        startline = 0
        endline = sep
        intype._process_subscript(lines, startline, endline)
        
        outtype = MsgType("", False)
        outtype.name = self.name
        outtype.cname = 'out_' + self.cname
        outtype.ctype = 'struct out_' + self.cname
        startline = sep + 1
        endline = len(lines) - 1
        outtype._process_subscript(lines, startline, endline)
        
        self.intype = intype
        self.outtype = outtype
    
    def gen_struct_in(self, comments=True):
        maxtype = 0
        maxname = 0
        for f in self.intype.fields:
            if f.arraylen == '*':
                typelen = len('UrosTcpRosArray')
            else:
                typelen = len(f.ctype)
            if maxtype < typelen:
                maxtype = typelen
            
            if f.arraylen and f.arraylen != '*':
                namelen = len('%s[%d]' % (f.cname, f.arraylen));
            else:
                namelen = len(f.cname)
            if maxname < namelen:
                maxname = namelen
        
        maxtype = next_tab(len(tab) + maxtype)
        text = '/**\n'
        line = ' * @brief'
        text += line + ' ' * (next_tab(len(line)) - len(line))
        text += 'TCPROS <tt>%s</tt> service request descriptor.\n' % self.name
        text += ' */\n'
        text += '%s {\n' % self.intype.ctype
        if len(self.intype.fields) > 0:
            text += self.intype.gen_struct_body(maxtype, maxname, comments)
        else:
            text += tab + '/* This message type has no fields.*/\n'
            text += tab + 'uint8_t _dummy;\n'
        text += '};'
        return text
    
    def gen_struct_out(self, comments=True):
        maxtype = 0
        maxname = 0
        for f in self.outtype.fields:
            if f.arraylen == '*':
                typelen = len('UrosTcpRosArray')
            else:
                typelen = len(f.ctype)
            if maxtype < typelen:
                maxtype = typelen
            
            if f.arraylen and f.arraylen != '*':
                namelen = len('%s[%d]' % (f.cname, f.arraylen));
            else:
                namelen = len(f.cname)
            if maxname < namelen:
                maxname = namelen
        
        maxtype = next_tab(len(tab) + maxtype)
        text = '/**\n'
        line = ' * @brief'
        text += line + ' ' * (next_tab(len(line)) - len(line))
        text += 'TCPROS <tt>%s</tt> service response descriptor.\n' % self.name
        text += ' */\n'
        text += '%s {\n' % self.outtype.ctype
        if len(self.outtype.fields) > 0:
            text += self.outtype.gen_struct_body(maxtype, maxname, comments)
        else:
            text += tab + '/* This message type has no fields.*/\n'
            text += tab + 'uint8_t _dummy;\n'
        text += '};'
        return text
    
    def gen_length_sig_in(self):
        text = 'size_t length_%s(\n' % self.intype.cname
        text += tab + '%s *objp\n' % self.intype.ctype
        text += ')'
        return text
    
    def gen_length_in(self):
        text = '/**\n'
        text += ' * @brief   Content length of a TCPROS <tt>%s</tt> service request.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Length of the TCPROS service request contents, in bytes.\n'
        text += ' */\n'
        text += self.gen_length_sig_in() + ' {\n'
        text += tab + 'size_t length = 0;\n'
        text += (tab + 'uint32_t i;\n\n') if self.intype.uses_arrays else '\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.intype.gen_length_body()
        if len(body) > 0:
            text += body + '\n'
            objpUsed = False
            for f in self.intype.fields:
                if f.rostype == 'string' or not primitive_map.has_key(f.rostype) or f.arraylen == '*':
                    objpUsed = True
                    break
            if not objpUsed:
                text += tab + '(void)objp;\n'
        else:
            text += tab + '/* Nothing to measure.*/\n'
            text += tab + '(void)objp;\n'
        text += tab + 'return length;\n'
        text += '}'
        return text
    
    def gen_length_sig_out(self):
        text = 'size_t length_%s(\n' % self.outtype.cname
        text += tab + '%s *objp\n' % self.outtype.ctype
        text += ')'
        return text
    
    def gen_length_out(self):
        text = '/**\n'
        text += ' * @brief   Content length of a TCPROS <tt>%s</tt> service response.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.ctype
        text += ' * @return\n'
        text += ' *          Length of the TCPROS service response contents, in bytes.\n'
        text += ' */\n'
        text += self.gen_length_sig_out() + ' {\n'
        text += tab + 'size_t length = 0;\n'
        text += (tab + 'uint32_t i;\n\n') if self.outtype.uses_arrays else '\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.outtype.gen_length_body()
        if len(body) > 0:
            text += body + '\n'
            objpUsed = False
            for f in self.outtype.fields:
                if f.rostype == 'string' or not primitive_map.has_key(f.rostype) or f.arraylen == '*':
                    objpUsed = True
                    break
            if not objpUsed:
                text += tab + '(void)objp;\n'
        else:
            text += tab + '/* Nothing to measure.*/\n'
            text += tab + '(void)objp;\n'
        text += tab + 'return length;\n'
        text += '}'
        return text
    
    def gen_init_sig_in(self):
        text = 'void init_%s(\n' % self.intype.cname
        text += tab + '%s *objp\n' % self.intype.ctype
        text += ')'
        return text
    
    def gen_init_in(self):
        text = '/**\n'
        text += ' * @brief   Initializes a TCPROS <tt>%s</tt> service request.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an allocated <code>%s</code> object.\n' % self.intype.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_init_sig_in() + ' {\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.intype.gen_init_body()
        if len(body) > 0:
            text += body
        else:
            text += tab + '/* Nothing to initialize.*/\n'
            text += tab + '(void)objp;\n'
        text += '}'
        return text
    
    def gen_init_sig_out(self):
        text = 'void init_%s(\n' % self.outtype.cname
        text += tab + '%s *objp\n' % self.outtype.ctype
        text += ')'
        return text
    
    def gen_init_out(self):
        text = '/**\n'
        text += ' * @brief   Initializes a TCPROS <tt>%s</tt> service request.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an allocated <code>%s</code> object.\n' % self.outtype.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_init_sig_out() + ' {\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.outtype.gen_init_body()
        if len(body) > 0:
            text += body
        else:
            text += tab + '/* Nothing to initialize.*/\n'
            text += tab + '(void)objp;\n'
        text += '}'
        return text
    
    def gen_clean_sig_in(self):
        text = 'void clean_%s(\n' % self.intype.cname
        text += tab + '%s *objp\n' % self.intype.ctype
        text += ')'
        return text
    
    def gen_clean_in(self):
        text = '/**\n'
        text += ' * @brief   Cleans a TCPROS <tt>%s</tt> service request.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code>object.\n' % self.intype.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_clean_sig_in() + ' {\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.outtype.gen_clean_body()
        if len(body) > 0:
            text += body
        else:
            text += tab + '/* Nothing to clean.*/\n'
            text += tab + '(void)objp;\n'
        text += '}'
        return text
    
    def gen_clean_sig_out(self):
        text = 'void clean_%s(\n' % self.outtype.cname
        text += tab + '%s *objp\n' % self.outtype.ctype 
        text += ')'
        return text
    
    def gen_clean_out(self):
        text = '/**\n'
        text += ' * @brief   Cleans a TCPROS <tt>%s</tt> service request.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.outtype.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_clean_sig_out() + ' {\n'
        text += tab + 'urosAssert(objp != NULL);\n\n'
        body = self.outtype.gen_clean_body()
        if len(body) > 0:
            text += body
        else:
            text += tab + '/* Nothing to clean.*/\n'
            text += tab + '(void)objp;\n'
        text += '}'
        return text
    
    def gen_recv_sig(self):
        text = 'uros_err_t recv_%s(\n' % self.intype.cname
        text += tab + 'UrosTcpRosStatus *tcpstp,\n'
        text += tab + '%s *objp\n' % self.intype.ctype
        text += ')'
        return text
    
    def gen_recv(self):
        text = '/**\n'
        text += ' * @brief   Receives a TCPROS <tt>%s</tt> service request.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @param[out] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.intype.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_recv_sig() + ' {\n'
        text += tab + 'urosAssert(tcpstp != NULL);\n'
        text += tab + 'urosAssert(urosConnIsValid(tcpstp->csp));\n'
        text += tab + 'urosAssert(objp != NULL);\n'
        body = self.intype.gen_recv_body()
        if len(body) > 0:
            text += '#define _CHKOK { if (tcpstp->err) { goto _error; } }\n\n'
            text += body
            text += '\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
            text += '_error:\n'
            text += tab + 'clean_%s(objp);\n' % self.intype.cname
            text += tab + 'return tcpstp->err;\n'
            text += '#undef _CHKOK\n'
        else:
            text += '\n'
            text += tab + '/* Nothing to receive.*/\n'
            text += tab + '(void)objp;\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
        text += '}'
        return text
    
    def gen_send_sig(self):
        text = 'uros_err_t send_%s(\n' % self.outtype.cname
        text += tab + 'UrosTcpRosStatus *tcpstp,\n'
        text += tab + '%s *objp\n' % self.outtype.ctype
        text += ')'
        return text
    
    def gen_send(self):
        text = '/**\n'
        text += ' * @brief   Sends a TCPROS <tt>%s</tt> service response.\n' % self.name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @param[in] objp\n'
        text += ' *          Pointer to an initialized <code>%s</code> object.\n' % self.outtype.ctype
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_send_sig() + ' {\n'
        text += tab + 'urosAssert(tcpstp != NULL);\n'
        text += tab + 'urosAssert(urosConnIsValid(tcpstp->csp));\n'
        text += tab + 'urosAssert(objp != NULL);\n'
        body = self.outtype.gen_send_body()
        if len(body) > 0:
            text += '#define _CHKOK { if (tcpstp->err) { return tcpstp->err; } }\n\n'
            text += body
            text += '\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
            text += '#undef _CHKOK\n'
        else:
            text += '\n'
            text += tab + '/* Nothing to send.*/\n'
            text += tab + '(void)objp;\n'
            text += tab + 'return tcpstp->err = UROS_OK;\n'
        text += '}'
        return text

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

class CodeGen:
    def __init__(self):
        # Options
        self.opts = {
            'author'                    : 'TODO',
            'licenseFile'               : '',
            
            'nodeName'                  : 'uros_node',
            
            'includeDir'                : '.',
            'sourceDir'                 : '.',
            'msgTypesFilename'          : 'urosMsgTypes',
            'handlersFilename'          : 'urosHandlers',
            'genMsgTypesHeader'         : 'true',
            'genMsgTypesSource'         : 'true',
            'genHandlersHeader'         : 'true',
            'genHandlersSource'         : 'true',
            
            'fieldComments'             : 'true',
            'msgOnStack'                : 'false',
            'inOnStack'                 : 'false',
            'outOnStack'                : 'false',
            'msgVarBaseName'            : 'msg',
            'inVarBaseName'             : 'inmsg',
            'outVarBaseName'            : 'outmsg',
            
            'regTypesFuncName'          : 'urosMsgTypesRegStaticTypes',
            'regPubTopicsFuncName'      : 'urosHandlersPublishTopics',
            'unregPubTopicsFuncName'    : 'urosHandlersUnpublishTopics',
            'regSubTopicsFuncName'      : 'urosHandlersSubscribeTopics',
            'unregSubTopicsFuncName'    : 'urosHandlersUnsubscribeTopics',
            'regPubServicesFuncName'    : 'urosHandlersPublishServices',
            'unregPubServicesFuncName'  : 'urosHandlersUnpublishServices',
        }
        self.pubTopics = {}
        self.subTopics = {}
        self.pubServices = {}
        self.callServices = {}
        
        # Internal objects
        self.cfgPath = None
        self.msgTypes = {}
        self.srvTypes = {}
        self.sortedMsgTypeNames = []
        self.licenseText = ""
    
    def _add_msgtype(self, deps, rostype):
        if not rostype in self.msgTypes:
            msgtype = MsgType(rostype)
            self.msgTypes[rostype] = msgtype
            
            subtypes = msgtype.get_complextypes()
            for k in subtypes:
                if not k in self.msgTypes:
                    self.msgTypes[k] = subtypes[k]
            
            msgdeps = msgtype.get_deps()
            for e in msgdeps:
                if not e in deps:
                    deps.append(e)
    
    def _add_srvtype(self, deps, rostype):
        if not rostype in self.srvTypes:
            srvtype = SrvType(rostype)
            self.srvTypes[rostype] = srvtype
            
            subtypes = srvtype.get_complextypes()
            for k in subtypes:
                if not k in self.msgTypes:
                    self.msgTypes[k] = subtypes[k]
            
            msgdeps = srvtype.get_deps(True)
            for e in msgdeps:
                if not e in deps:
                    deps.append(e)
    
    def elaborate(self):
        self.msgTypes = {}
        self.srvTypes = {}
        self.sortedMsgTypeNames = []
        deps = []
        
        for name in self.pubTopics:
            self._add_msgtype(deps, self.pubTopics[name])
        
        for name in self.subTopics:
            self._add_msgtype(deps, self.subTopics[name])
        
        for name in self.pubServices:
            self._add_srvtype(deps, self.pubServices[name])
        
        for name in self.callServices:
            self._add_srvtype(deps, self.callServices[name])
        
        self.sortedMsgTypeNames = sorted_deps(deps)
        for name in self.pubTopics:
            rostype = self.pubTopics[name]
            if not rostype in self.sortedMsgTypeNames:
                self.sortedMsgTypeNames.append(rostype)
        for name in self.subTopics:
            rostype = self.subTopics[name]
            if not rostype in self.sortedMsgTypeNames:
                self.sortedMsgTypeNames.append(rostype)
    
        self.cfgPath = os.sep.join(os.path.split(self.cfgPath)[:-1])
        if len(self.cfgPath) == 0: self.cfgPath = '.'
        
        self.licenseText = ""
        if len(self.opts['licenseFile']) > 0:
            licpath = os.path.normpath(self.cfgPath + os.sep + self.opts['licenseFile'])
            if os.path.exists(licpath):
                with open(licpath, 'r') as f:
                    self.licenseText = '/*\n' + (''.join(f.readlines())).strip() + '\n*/\n\n'
            else:
                raise ValueError('Invalid license file path: [%s]' % licpath)
        
        hpath = os.sep.join(os.path.split(self.msgtypes_header_path())[:-1])
        if len(hpath) == 0: hpath = '.'
        if not os.path.exists(hpath):
            raise EnvironmentError('The header output path [%s] does not exist' % hpath)
        
        cpath = os.sep.join(os.path.split(self.msgtypes_source_path())[:-1])
        if len(cpath) == 0: cpath = '.'
        if not os.path.exists(cpath):
            raise EnvironmentError('The source output path [%s] does not exist' % cpath)
    
    def load(self, cfgPath):
        self.__init__()
        
        self.cfgPath = cfgPath
        if cfgPath == '.':
            lines = sys.stdin.readlines()
        else:
            if os.path.exists(cfgPath):
                with open(cfgPath, 'r') as f:
                    lines = f.readlines()
            else:
                raise ValueError('Invalid CFG file path: [%s]' % cfgPath)
        
        modes = [ '[options]', '[pubtopics]', '[subtopics]', '[pubservices]', '[callservices]' ]
        mode = None
        modeidx = -1
        for line in lines:
            line = (line.partition('#'))[0].strip()
            if len(line) == 0: continue
            if line.lower() in modes:
                mode = line.lower()
                if modes.index(mode) != modeidx + 1:
                    text = 'Sections must be in the order:\n'
                    for m in modes: text += tab + m + '\n'
                    raise ValueError(text)
                modeidx += 1
                continue
            
            if not '=' in line:
                raise ValueError("Cannot find '=' in line: [%s]" % line)
            eqidx = line.index('=')
            key = line[:eqidx].strip()
            value = line[eqidx+1:].strip()
            
            if mode == '[options]':
                if len(value) >= 2 and \
                   ((value[0] == '"' and value[-1] == '"') or \
                    (value[0] == "'" and value[-1] == "'")):
                    value = value[1:-1]
                if not self.opts.has_key(key):
                    raise ValueError('Invalid option: ' + key)
                self.opts[key] = value
            
            else:
                if key[0] == '~':
                    if not valid_name(self.opts['nodeName']):
                        raise ValueError('[%s] is not a valid node name' % self.opts['nodeName']) 
                    key = key[1:]
                    key = self.opts['nodeName'] + '/' + key
                elif key[0] == '/':
                    key = key[1:]
                if not valid_path(key):
                    raise ValueError('[/%s] is not a valid ROS topic/service path' % key)
                if not valid_path(value):
                    raise ValueError('[/%s] is not a valid ROS type path' % value)
                key = '/' + key
                
                if mode == '[pubtopics]' and not key in self.pubTopics:
                    self.pubTopics[key] = value
                    
                elif mode == '[subtopics]' and not key in self.subTopics:
                    self.subTopics[key] = value
                    
                elif mode == '[pubservices]' and not key in self.pubServices:
                    self.pubServices[key] = value
                    
                elif mode == '[callservices]' and not key in self.callServices:
                    self.callServices[key] = value
        
        if modeidx != len(modes) - 1:
            raise ValueError('Not all the sections were defined')
            
    def gen_typereg_sig(self):
        return 'void %s(void)' % self.opts['regTypesFuncName']
        
    def gen_typereg_func(self):
        text = '/**\n'
        text += ' * @brief   Static TCPROS types registration.\n'
        text += ' * @details Statically registers all the TCPROS message and service types used\n'
        text += ' *          within this source file.\n'
        text += ' * @note    Should be called by @p urosUserRegisterStaticMsgTypes().\n'
        text += ' * @see     urosUserRegisterStaticMsgTypes()\n'
        text += ' */\n'
        text += self.gen_typereg_sig() + ' {\n'
        
        if len(self.msgTypes) > 0:
            text += '\n'
            text += tab + '/* MESSAGE TYPES */\n'
        for name in sorted(self.msgTypes):
            msgtype = self.msgTypes[name]
            text += '\n'
            text += tab + '/* %s */\n' % msgtype.name
            text += tab + 'urosRegisterStaticMsgTypeSZ("%s",\n' % msgtype.name
            text += tab + '                            NULL, "%s");\n' % msgtype.md5str
        
        if len(self.srvTypes) > 0:
            text += '\n'
            text += tab + '/* SERVICE TYPES */\n'
        for name in sorted(self.srvTypes):
            srvtype = self.srvTypes[name]
            text += '\n'
            text += tab + '/* %s */\n' % srvtype.name
            text += tab + 'urosRegisterStaticSrvTypeSZ("%s",\n' % srvtype.name
            text += tab + '                            NULL, "%s");\n' % srvtype.md5str
        
        text += '}'
        return text
    
    def gen_msgtypes_header(self):
        fileupr = '_' + self.opts['msgTypesFilename'].upper() + '_H_'
        comments = str2bool(self.opts['fieldComments'])
        
        text = self.licenseText
        text += '/**\n'
        text += ' * @file    %s.h\n' % self.opts['msgTypesFilename']
        text += ' * @author  %s\n' % self.opts['author']
        text += ' *\n'
        text += ' * @brief   TCPROS message and service descriptors.\n'
        text += ' */\n\n'
        text += '#ifndef %s\n#define %s\n\n' % (fileupr, fileupr)
        text += banner_big('HEADER FILES') + '\n\n'
        text += '#include <urosTcpRos.h>\n\n'
        text += '#ifdef __cplusplus\n'
        text += 'extern "C" {\n'
        text += '#endif\n\n'
        text += banner_big(' MESSAGE TYPES') + '\n\n'
        text += '/** @addtogroup tcpros_msg_types */\n/** @{ */\n\n'
        
        for name in self.sortedMsgTypeNames:
            msgtype = self.msgTypes[name]
            text += banner_small('MESSAGE: ' + msgtype.name) + '\n\n'
            text += msgtype.gen_struct(comments) + '\n\n'
        if len(self.sortedMsgTypeNames) == 0:
            text += '/* There are no message types.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('SERVICE TYPES') + '\n\n'
        text += '/** @addtogroup tcpros_srv_types */\n/** @{ */\n\n'
        
        for name in self.srvTypes:
            srvtype = self.srvTypes[name]
            text += banner_small('SERVICE: ' + srvtype.name) + '\n\n'
            text += srvtype.gen_struct_in(comments) + '\n\n'
            text += srvtype.gen_struct_out(comments) + '\n\n'
        if len(self.srvTypes) == 0:
            text += '/* There are no service types.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('MESSAGE CONSTANTS') + '\n\n'
        text += '/** @addtogroup tcpros_msg_consts */\n/** @{ */\n\n'
        
        found = False
        for name in self.sortedMsgTypeNames:
            msgtype = self.msgTypes[name]
            decls = msgtype.gen_const_decls(comments)
            if len(decls) > 0:
                found = True
                text += banner_small('MESSAGE: ' + msgtype.name) + '\n\n'
                text += '/** @name Message <tt>%s</tt> */\n/** @{ */\n\n' % msgtype.name
                text += decls
                text += '/** @} */\n\n'
        if not found:
            text += '/* There are no message costants.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('SERVICE CONSTANTS') + '\n\n'
        text += '/** @addtogroup tcpros_srv_consts */\n/** @{ */\n\n'
        
        found = False
        for name in self.srvTypes:
            srvtype = self.srvTypes[name]
            indecls = srvtype.intype.gen_const_decls(comments)
            outdecls = srvtype.outtype.gen_const_decls(comments)
            if len(indecls) > 0 or len(outdecls) > 0:
                found = True
                text += banner_small('SERVICE: ' + srvtype.name) + '\n\n'
                text += '/** @name Service <tt>%s</tt> */\n/** @{ */\n\n' % srvtype.name
                text += indecls + outdecls
                text += '/** @} */\n\n'
        if not found:
            text += '/* There are no service costants.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('MESSAGE PROTOTYPES') + '\n\n'
        
        for name in self.sortedMsgTypeNames:
            msgtype = self.msgTypes[name]
            text += banner_small('MESSAGE: ' + msgtype.name) + '\n\n'
            text += msgtype.gen_length_sig() + ';\n'
            text += msgtype.gen_init_sig() + ';\n'
            text += msgtype.gen_clean_sig() + ';\n'
            text += msgtype.gen_recv_sig() + ';\n'
            text += msgtype.gen_send_sig() + ';\n\n'
        if len(self.sortedMsgTypeNames) == 0:
            text += '/* There are no message types.*/\n\n'
        
        text += banner_big('SERVICE PROTOTYPES') + '\n\n'
        for name in self.srvTypes:
            srvtype = self.srvTypes[name]
            text += banner_small('SERVICE: ' + msgtype.name) + '\n\n'
            text += srvtype.gen_length_sig_in() + ';\n'
            text += srvtype.gen_length_sig_out() + ';\n'
            text += srvtype.gen_init_sig_in() + ';\n'
            text += srvtype.gen_init_sig_out() + ';\n'
            text += srvtype.gen_clean_sig_in() + ';\n'
            text += srvtype.gen_clean_sig_out() + ';\n'
            text += srvtype.gen_recv_sig() + ';\n'
            text += srvtype.gen_send_sig() + ';\n\n'
        if len(self.srvTypes) == 0:
            text += '/* There are no service types.*/\n\n'
        
        text += banner_big('GLOBAL PROTOTYPES') + '\n\n'
        text += self.gen_typereg_sig() + ';\n\n'
        
        text += '#ifdef __cplusplus\n'
        text += '} /* extern "C" */\n'
        text += '#endif\n\n'
        text += '#endif /* %s */\n\n' % fileupr
        return text
    
    def msgtypes_header_path(self):
        path = self.cfgPath + os.sep + self.opts['includeDir'];
        return os.path.normpath(path + os.sep + self.opts['msgTypesFilename'] + '.h')
    
    def export_msgtypes_header(self, text):
        with open(self.msgtypes_header_path(), 'w') as f:
            f.write(text)
    
    def gen_msgtypes_source(self):
        comments = str2bool(self.opts['fieldComments'])
        
        text = self.licenseText
        text += '/**\n'
        text += ' * @file    %s.c\n' % self.opts['msgTypesFilename']
        text += ' * @author  %s\n' % self.opts['author']
        text += ' *\n'
        text += ' * @brief   TCPROS message and service descriptor functions.\n'
        text += ' */\n\n'
        text += banner_big('HEADER FILES') + '\n\n'
        text += '#include "%s.h"\n\n' % self.opts['msgTypesFilename']
        text += banner_big('MESSAGE CONSTANTS') + '\n\n'
        text += '/** @addtogroup tcpros_msg_consts */\n/** @{ */\n\n'
        
        found = False
        for name in self.sortedMsgTypeNames:
            msgtype = self.msgTypes[name]
            defs = msgtype.gen_const_defs(comments)
            if len(defs) > 0:
                found = True
                text += banner_small('MESSAGE: ' + name) + '\n\n'
                text += '/** @name Message <tt>%s</tt> */\n/** @{ */\n\n' % name
                text += defs
                text += '/** @} */\n\n'
        if not found:
            text += '/* There are no message constants.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('SERVICE CONSTANTS') + '\n\n'
        text += '/** @addtogroup tcpros_srv_consts */\n/** @{ */\n\n'
        
        found = False
        for name in self.srvTypes:
            srvtype = self.srvTypes[name]
            indefs = srvtype.intype.gen_const_defs(comments)
            outdefs = srvtype.outtype.gen_const_defs(comments)
            if len(indefs) > 0 or len(outdefs) > 0:
                found = True
                text += banner_small('SERVICE: ' + name) + '\n\n'
                text += '/** @name Service <tt>%s</tt> */\n/** @{ */\n\n' % name
                text += indefs + outdefs
                text += '/** @} */\n\n'
        if not found:
            text += '/* There are no service constants.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('MESSAGE FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_msg_funcs */\n/** @{ */\n\n'
        
        for name in self.sortedMsgTypeNames:
            msgtype = self.msgTypes[name]
            text += banner_small('MESSAGE: ' + name) + '\n\n'
            text += '/** @name Message <tt>%s</tt> */\n/** @{ */\n\n' % name
            text += msgtype.gen_length() + '\n\n'
            text += msgtype.gen_init() + '\n\n'
            text += msgtype.gen_clean() + '\n\n'
            text += msgtype.gen_recv() + '\n\n'
            text += msgtype.gen_send() + '\n\n'
            text += '/** @} */\n\n'
        if len(self.sortedMsgTypeNames) == 0:
            text += '/* There are no message types.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('SERVICE FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_srv_funcs */\n/** @{ */\n\n'
        
        for name in self.srvTypes:
            srvtype = self.srvTypes[name]
            text += banner_small('SERVICE: ' + name) + '\n\n'
            text += '/** @name Service <tt>%s</tt> */\n/** @{ */\n\n' % name
            text += srvtype.gen_length_in() + '\n\n'
            text += srvtype.gen_init_in() + '\n\n'
            text += srvtype.gen_clean_in() + '\n\n'
            text += srvtype.gen_length_out() + '\n\n'
            text += srvtype.gen_init_out() + '\n\n'
            text += srvtype.gen_clean_out() + '\n\n'
            text += srvtype.gen_recv() + '\n\n'
            text += srvtype.gen_send() + '\n\n'
            text += '/** @} */\n\n'
        if len(self.srvTypes) == 0:
            text += '/* There are no service types.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('GLOBAL FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_funcs */\n/** @{ */\n\n'
        text += self.gen_typereg_func() + '\n\n'
        text += '/** @} */\n\n'
        return text
    
    def msgtypes_source_path(self):
        path = self.cfgPath + os.sep + self.opts['sourceDir'];
        return os.path.normpath(path + os.sep + self.opts['msgTypesFilename'] + '.c')
        
    def export_msgtypes_source(self, text):
        with open(self.msgtypes_source_path(), 'w') as f:
            f.write(text)
    
    def gen_pubtopic_sig(self, name):
        return 'uros_err_t pub_tpc%s(UrosTcpRosStatus *tcpstp)' % mangled_name(name)
        
    def gen_pubtopic_handler(self, name):
        msgtype = self.msgTypes[self.pubTopics[name]]
        onstack = str2bool(self.opts['msgOnStack'])
        if onstack:
            msgref = '&' + self.opts['msgVarBaseName']
            msgshchar = 'S'
        else:
            msgref = self.opts['msgVarBaseName'] + 'p'
            msgshchar = 'H'
        
        text = '/**\n'
        text += ' * @brief   TCPROS <tt>%s</tt> published topic handler.\n' % name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_pubtopic_sig(name) + ' {\n\n'
        text += tab + '/* Message allocation and initialization.*/\n'
        text += tab + 'UROS_TPC_INIT_%s(%s);\n\n' % (msgshchar, msgtype.cname)
        text += tab + '/* Published messages loop.*/\n'
        text += tab + 'while (!urosTcpRosStatusCheckExit(tcpstp)) {\n'
        text += tab*2 + '/* TODO: Generate the contents of the message.*/\n'
        text += tab*2 + 'urosThreadSleepSec(1); continue; /* TODO: Remove this dummy line.*/\n\n'
        text += tab*2 + '/* Send the message.*/\n'
        text += tab*2 + 'UROS_MSG_SEND_LENGTH(%s, %s);\n' % (msgref, msgtype.cname)
        text += tab*2 + 'UROS_MSG_SEND_BODY(%s, %s);\n\n' % (msgref, msgtype.cname)
        text += tab*2 + '/* Dispose the contents of the message.*/\n'
        text += tab*2 + 'clean_%s(%s);\n' % (msgtype.cname, msgref)
        text += tab + '}\n'
        text += tab + 'tcpstp->err = UROS_OK;\n\n'
        text += '_finally:\n'
        text += tab + '/* Message deinitialization and deallocation.*/\n'
        text += tab + 'UROS_TPC_UNINIT_%s(%s);\n' % (msgshchar, msgtype.cname)
        text += tab + 'return tcpstp->err;\n'
        text += '}'
        return text
    
    def gen_subtopic_sig(self, name):
        return 'uros_err_t sub_tpc%s(UrosTcpRosStatus *tcpstp)' % mangled_name(name)

    def gen_subtopic_cb_sig(self, name):
        msgtype = self.msgTypes[self.subTopics[name]]
        return 'void sub_cb%s(struct %s *msg)' % (mangled_name(name), msgtype.cname)

    def gen_subtopic_cb(self, name):
        msgtype = self.msgTypes[self.subTopics[name]]
        return 'sub_cb%s(&msg)' % mangled_name(name)
    
    def gen_subtopic_handler(self, name):
        msgtype = self.msgTypes[self.subTopics[name]]
        onstack = str2bool(self.opts['msgOnStack'])
        if onstack:
            msgref = '&' + self.opts['msgVarBaseName']
            msgshchar = 'S'
        else:
            msgref = self.opts['msgVarBaseName'] + 'p'
            msgshchar = 'H'
        
        text = '/**\n'
        text += ' * @brief   TCPROS <tt>%s</tt> subscribed topic handler.\n' % name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_subtopic_sig(name) + ' {\n\n'
        text += tab + '/* Message allocation and initialization.*/\n'
        text += tab + 'UROS_TPC_INIT_%s(%s);\n\n' % (msgshchar, msgtype.cname)
        text += tab + '/* Subscribed messages loop.*/\n'
        text += tab + 'while (!urosTcpRosStatusCheckExit(tcpstp)) {\n'
        text += tab*2 + '/* Receive the next message.*/\n'
        text += tab*2 + 'UROS_MSG_RECV_LENGTH();\n'
        text += tab*2 + 'UROS_MSG_RECV_BODY(%s, %s);\n\n' % (msgref, msgtype.cname)
        text += tab*2 + '/* Call callback function.*/\n'
        text += tab*2 + self.gen_subtopic_cb(name) + ';\n\n'
        text += tab*2 + '/* Dispose the contents of the message.*/\n'
        text += tab*2 + 'clean_%s(%s);\n' % (msgtype.cname, msgref)
        text += tab + '}\n'
        text += tab + 'tcpstp->err = UROS_OK;\n\n'
        text += '_finally:\n'
        text += tab + '/* Message deinitialization and deallocation.*/\n'
        text += tab + 'UROS_TPC_UNINIT_%s(%s);\n' % (msgshchar, msgtype.cname)
        text += tab + 'return tcpstp->err;\n'
        text += '}'
        return text
    
    def gen_pubservice_sig(self, name):
        return 'uros_err_t pub_srv%s(UrosTcpRosStatus *tcpstp)' % mangled_name(name)
    
    def gen_pubservice_handler(self, name):
        srvtype = self.srvTypes[self.pubServices[name]]
        inonstack = str2bool(self.opts['inOnStack'])
        if inonstack:
            inref = '&' + self.opts['inVarBaseName']
            inshchar = 'S'
        else:
            inref = self.opts['inVarBaseName'] + 'p'
            inshchar = 'H'
        outonstack = str2bool(self.opts['outOnStack'])
        if outonstack:
            outref = '&' + self.opts['outVarBaseName']
            outshchar = 'S'
        else:
            outref = self.opts['outVarBaseName'] + 'p'
            outshchar = 'H'
        
        text = '/**\n'
        text += ' * @brief   TCPROS <tt>%s</tt> published service handler.\n' % name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_pubservice_sig(name) + ' {\n\n'
        text += tab + '/* Service messages allocation and initialization.*/\n'
        text += tab + 'UROS_SRV_INIT_%sI%sO(%s,\n' % (inshchar, outshchar, srvtype.intype.cname)
        text += tab + '                   %s);\n\n' % srvtype.outtype.cname
        text += tab + '/* Service message loop (if the service is persistent).*/\n'
        text += tab + 'do {\n'
        text += tab*2 + '/* Receive the request message.*/\n'
        text += tab*2 + 'UROS_MSG_RECV_LENGTH();\n'
        text += tab*2 + 'UROS_MSG_RECV_BODY(%s, %s);\n\n' % (inref, srvtype.intype.cname)
        text += tab*2 + '/* TODO: Process the request message.*/\n'
        text += tab*2 + 'tcpstp->err = UROS_OK;\n'
        text += tab*2 + 'urosStringClean(&tcpstp->errstr);\n'
        text += tab*2 + 'okByte = 1;\n\n'
        text += tab*2 + '/* Dispose the contents of the request message.*/\n'
        text += tab*2 + 'clean_%s(%s);\n\n' % (srvtype.intype.cname, inref)
        text += tab*2 + '/* TODO: Generate the contents of the response message.*/\n\n'
        text += tab*2 + '/* Send the response message.*/\n'
        text += tab*2 + 'UROS_SRV_SEND_OKBYTE_ERRSTR();\n'
        text += tab*2 + 'UROS_MSG_SEND_LENGTH(%s, %s);\n' % (outref, srvtype.outtype.cname)
        text += tab*2 + 'UROS_MSG_SEND_BODY(%s, %s);\n\n' % (outref, srvtype.outtype.cname)
        text += tab*2 + '/* Dispose the contents of the response message.*/\n'
        text += tab*2 + 'clean_%s(%s);\n' % (srvtype.outtype.cname, outref)
        text += tab + '} while (tcpstp->topicp->flags.persistent &&\n'
        text += tab + '         !urosTcpRosStatusCheckExit(tcpstp));\n'
        text += tab + 'tcpstp->err = UROS_OK;\n\n'
        text += '_finally:\n'
        text += tab + '/* Service messages deinitialization and deallocation.*/\n'
        text += tab + 'UROS_SRV_UNINIT_%sI%sO(%s,\n' % (inshchar, outshchar, srvtype.intype.cname)
        text += tab + '                     %s);\n' % srvtype.outtype.cname
        text += tab + 'return tcpstp->err;\n'
        text += '}'
        return text
    
    def gen_callservice_sig(self, name):
        srvtype = self.srvTypes[self.pubServices[name]]
        text =  'uros_err_t call_srv%s(\n' % mangled_name(name)
        text += tab + 'UrosTcpRosStatus *tcpstp,\n'
        text += tab + '%s *%sp,\n' % (srvtype.intype.ctype, self.opts['inVarBaseName'])
        text += tab + '%s *%sp\n' % (srvtype.outtype.ctype, self.opts['outVarBaseName'])
        text += ')'
        return text
    
    def gen_callservice_handler(self, name):
        srvtype = self.srvTypes[self.pubServices[name]]
        inref = self.opts['inVarBaseName'] + 'p'
        outref = self.opts['outVarBaseName'] + 'p'
        
        text = '/**\n'
        text += ' * @brief   TCPROS <tt>%s</tt> called service handler.\n' % name
        text += ' *\n'
        text += ' * @param[in,out] tcpstp\n'
        text += ' *          Pointer to a working @p UrosTcpRosStatus object.\n'
        text += ' * @param[in] inmsgp\n'
        text += ' *          Pointer to the initialized request message.\n'
        text += ' * @param[out] outmsgp\n'
        text += ' *          Pointer to the allocated response message. It will be initialized\n'
        text += ' *          by this function. The service result will be written there only\n'
        text += ' *          if the call is successful.\n'
        text += ' * @return\n'
        text += ' *          Error code.\n'
        text += ' */\n'
        text += self.gen_callservice_sig(name) + ' {\n\n'
        text += tab + '/* Service messages allocation and initialization.*/\n'
        text += tab + 'UROS_SRVCALL_INIT(%s,\n' % srvtype.intype.cname
        text += tab + '                  %s);\n\n' % srvtype.outtype.cname
        text += tab + '/* Send the request message.*/\n'
        text += tab + 'UROS_MSG_SEND_LENGTH(%s, %s);\n' % (inref, srvtype.intype.cname)
        text += tab + 'UROS_MSG_SEND_BODY(%s, %s);\n\n' % (inref, srvtype.intype.cname)
        text += tab + '/* TODO: Dispose the contents of the request message.*/\n\n'
        text += tab + '/* Receive the response message.*/\n'
        text += tab + 'UROS_SRV_RECV_OKBYTE();\n'
        text += tab + 'UROS_MSG_RECV_LENGTH();\n'
        text += tab + 'UROS_MSG_RECV_BODY(%s, %s);\n\n' % (outref, srvtype.outtype.cname)
        text += tab + 'tcpstp->err = UROS_OK;\n'
        text += '_finally:\n'
        text += tab + 'return tcpstp->err;\n'
        text += '}'
        return text
            
    def gen_regpubtopics_sig(self):
        return 'void %s(void)' % self.opts['regPubTopicsFuncName']
        
    def gen_regpubtopics_func(self):
        text = '/**\n'
        text += ' * @brief   Registers all the published topics to the Master node.\n'
        text += ' * @note    Should be called at node initialization.\n'
        text += ' */\n'
        text += self.gen_regpubtopics_sig() + ' {\n\n'
        
        if len(self.pubTopics) > 0:
            for name in sorted(self.pubTopics):
                rostype = self.pubTopics[name];
                text += tab + '/* %s */\n' % name
                text += tab + 'urosNodePublishTopicSZ(\n'
                text += tab*2 + '"%s",\n' % name
                text += tab*2 + '"%s",\n' % rostype
                text += tab*2 + '(uros_proc_f)pub_tpc%s,\n' % mangled_name(name)
                text += tab*2 + 'uros_nulltopicflags\n'
                text += tab + ');\n\n'
            text = text[:-1]
        else:
            text += tab + '/* No topics to publish.*/\n'
            
        text += '}'
        return text
            
    def gen_unregpubtopics_sig(self):
        return 'void %s(void)' % self.opts['unregPubTopicsFuncName']
        
    def gen_unregpubtopics_func(self):
        text = '/**\n'
        text += ' * @brief   Unregisters all the published topics to the Master node.\n'
        text += ' * @note    Should be called at node shutdown.\n'
        text += ' */\n'
        text += self.gen_unregpubtopics_sig() + ' {\n\n'
        
        if len(self.pubTopics) > 0:
            for name in sorted(self.pubTopics):
                text += tab + '/* %s */\n' % name
                text += tab + 'urosNodeUnpublishTopicSZ(\n'
                text += tab*2 + '"%s"\n' % name
                text += tab + ');\n\n'
            text = text[:-1]
        else:
            text += tab + '/* No topics to unpublish.*/\n'
            
        text += '}'
        return text
            
    def gen_regsubtopics_sig(self):
        return 'void %s(void)' % self.opts['regSubTopicsFuncName']
        
    def gen_regsubtopics_func(self):
        text = '/**\n'
        text += ' * @brief   Registers all the subscribed topics to the Master node.\n'
        text += ' * @note    Should be called at node initialization.\n'
        text += ' */\n'
        text += self.gen_regsubtopics_sig() + ' {\n\n'
        
        if len(self.subTopics) > 0:
            for name in sorted(self.subTopics):
                rostype = self.subTopics[name];
                text += tab + '/* %s */\n' % name
                text += tab + 'urosNodeSubscribeTopicSZ(\n'
                text += tab*2 + '"%s",\n' % name
                text += tab*2 + '"%s",\n' % rostype
                text += tab*2 + '(uros_proc_f)sub_tpc%s,\n' % mangled_name(name)
                text += tab*2 + 'uros_nulltopicflags\n'
                text += tab + ');\n\n'
            text = text[:-1]
        else:
            text += tab + '/* No topics to subscribe to.*/\n'
            
        text += '}'
        return text
            
    def gen_unregsubtopics_sig(self):
        return 'void %s(void)' % self.opts['unregSubTopicsFuncName']
        
    def gen_unregsubtopics_func(self):
        text = '/**\n'
        text += ' * @brief   Unregisters all the subscribed topics to the Master node.\n'
        text += ' * @note    Should be called at node shutdown.\n'
        text += ' */\n'
        text += self.gen_unregsubtopics_sig() + ' {\n\n'
        
        if len(self.subTopics) > 0:
            for name in sorted(self.subTopics):
                text += tab + '/* %s */\n' % name
                text += tab + 'urosNodeUnsubscribeTopicSZ(\n'
                text += tab*2 + '"%s"\n' % name
                text += tab + ');\n\n'
            text = text[:-1]
        else:
            text += tab + '/* No topics to unsubscribe from.*/\n'
            
        text += '}'
        return text
            
    def gen_regpubservices_sig(self):
        return 'void %s(void)' % self.opts['regPubServicesFuncName']
        
    def gen_regpubservices_func(self):
        text = '/**\n'
        text += ' * @brief   Registers all the published services to the Master node.\n'
        text += ' * @note    Should be called at node initialization.\n'
        text += ' */\n'
        text += self.gen_regpubservices_sig() + ' {\n\n'
        
        if len(self.pubServices) > 0:
            for name in sorted(self.pubServices):
                rostype = self.pubServices[name];
                text += tab + '/* %s */\n' % name
                text += tab + 'urosNodePublishServiceSZ(\n'
                text += tab*2 + '"%s",\n' % name
                text += tab*2 + '"%s",\n' % rostype
                text += tab*2 + '(uros_proc_f)pub_srv%s,\n' % mangled_name(name)
                text += tab*2 + 'uros_nullserviceflags\n'
                text += tab + ');\n\n'
            text = text[:-1]
        else:
            text += tab + '/* No services to publish.*/\n'
            
        text += '}'
        return text
            
    def gen_unregpubservices_sig(self):
        return 'void %s(void)' % self.opts['unregPubServicesFuncName']
        
    def gen_unregpubservices_func(self):
        text = '/**\n'
        text += ' * @brief   Unregisters all the published services to the Master node.\n'
        text += ' * @note    Should be called at node shutdown.\n'
        text += ' */\n'
        text += self.gen_unregpubservices_sig() + ' {\n\n'
        
        if len(self.pubServices) > 0:
            for name in sorted(self.pubServices):
                text += tab + '/* %s */\n' % name
                text += tab + 'urosNodeUnpublishServiceSZ(\n'
                text += tab*2 + '"%s"\n' % name
                text += tab + ');\n\n'
            text = text[:-1]
        else:
            text += tab + '/* No services to unpublish.*/\n'
            
        text += '}'
        return text
    
    def gen_handlers_header(self):
        fileupr = '_' + self.opts['handlersFilename'].upper() + '_H_'
        
        text = self.licenseText
        text += '/**\n'
        text += ' * @file    %s.h\n' % self.opts['handlersFilename']
        text += ' * @author  %s\n' % self.opts['author']
        text += ' *\n'
        text += ' * @brief   TCPROS topic and service handlers.\n'
        text += ' */\n\n'
        text += '#ifndef %s\n#define %s\n\n' % (fileupr, fileupr)
        text += banner_big('HEADER FILES') + '\n\n'
        text += '#include "%s.h"\n\n' % self.opts['msgTypesFilename']
        text += '#ifdef __cplusplus\n'
        text += 'extern "C" {\n'
        text += '#endif\n\n'
        text += banner_big('PUBLISHED TOPIC PROTOTYPES') + '\n\n'
        
        for name in sorted(self.pubTopics):
            text += banner_small('PUBLISHED TOPIC: ' + name) + '\n\n'
            text += self.gen_pubtopic_sig(name) + ';\n\n'
        if len(self.pubTopics) == 0:
            text += '/* There are no published topics.*/\n\n'
        
        text += banner_big('SUBSCRIBED TOPIC PROTOTYPES') + '\n\n'
        
        for name in sorted(self.subTopics):
            text += banner_small('SUBSCRIBED TOPIC: ' + name) + '\n\n'
            text += self.gen_subtopic_sig(name) + ';\n\n'
            text += self.gen_subtopic_cb_sig(name) + ';\n\n'
        if len(self.subTopics) == 0:
            text += '/* There are no subscribed topics.*/\n\n'
        
        text += banner_big('PUBLISHED SERVICE PROTOTYPES') + '\n\n'
        
        for name in sorted(self.pubServices):
            text += banner_small('PUBLISHED SERVICE: ' + name) + '\n\n'
            text += self.gen_pubservice_sig(name) + ';\n\n'
        if len(self.pubServices) == 0:
            text += '/* There are no published services.*/\n\n'
        
        text += banner_big('CALLED SERVICE PROTOTYPES') + '\n\n'
        
        for name in sorted(self.callServices):
            text += banner_small('CALLED SERVICE: ' + name) + '\n\n'
            text += self.gen_callservice_sig(name) + ';\n\n'
        if len(self.callServices) == 0:
            text += '/* There are no called services.*/\n\n'
        
        text += banner_big('GLOBAL PROTOTYPES') + '\n\n'
        text += self.gen_regpubtopics_sig() + ';\n'
        text += self.gen_unregpubtopics_sig() + ';\n\n'
        text += self.gen_regsubtopics_sig() + ';\n'
        text += self.gen_unregsubtopics_sig() + ';\n\n'
        text += self.gen_regpubservices_sig() + ';\n'
        text += self.gen_unregpubservices_sig() + ';\n\n'
        
        text += '#ifdef __cplusplus\n'
        text += '} /* extern "C" */\n'
        text += '#endif\n\n'
        text += '#endif /* %s */\n\n' % fileupr
        return text
    
    def handlers_header_path(self):
        path = self.cfgPath + os.sep + self.opts['includeDir'];
        return os.path.normpath(path + os.sep + self.opts['handlersFilename'] + '.h')
        
    def export_handlers_header(self, text):
        with open(self.handlers_header_path(), 'w') as f:
            f.write(text)
    
    def gen_handlers_source(self):
        text = self.licenseText
        text += '/**\n'
        text += ' * @file    %s.c\n' % self.opts['handlersFilename']
        text += ' * @author  %s\n' % self.opts['author']
        text += ' *\n'
        text += ' * @brief   TCPROS topic and service handlers.\n'
        text += ' */\n\n'
        text += banner_big('HEADER FILES') + '\n\n'
        text += '#include "%s.h"\n\n' % self.opts['handlersFilename']
        text += '#include <urosNode.h>\n'
        text += '#include <urosTcpRos.h>\n'
        text += '#include <urosUser.h>\n\n'
        text += banner_big('PUBLISHED TOPIC FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_pubtopic_funcs */\n/** @{ */\n\n'
        
        for name in sorted(self.pubTopics):
            text += banner_small('PUBLISHED TOPIC: ' + name) + '\n\n'
            text += '/** @name Topic <tt>%s</tt> publisher */\n/** @{ */\n\n' % name
            text += self.gen_pubtopic_handler(name) + '\n\n'
            text += '/** @} */\n\n'
        if len(self.pubTopics) == 0:
            text += '/* There are no published topics.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('SUBSCRIBED TOPIC FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_subtopic_funcs */\n/** @{ */\n\n'
        
        for name in sorted(self.subTopics):
            text += banner_small('SUBSCRIBED TOPIC: ' + name) + '\n\n'
            text += '/** @name Topic <tt>%s</tt> subscriber */\n/** @{ */\n\n' % name
            text += self.gen_subtopic_handler(name) + '\n\n'
            text += '/** @} */\n\n'
        if len(self.subTopics) == 0:
            text += '/* There are no subscribed topics.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('PUBLISHED SERVICE FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_pubservice_funcs */\n/** @{ */\n\n'
        
        for name in sorted(self.pubServices):
            text += banner_small('PUBLISHED SERVICE: ' + name) + '\n\n'
            text += '/** @name Service <tt>%s</tt> publisher */\n/** @{ */\n\n' % name
            text += self.gen_pubservice_handler(name) + '\n\n'
            text += '/** @} */\n\n'
        if len(self.pubServices) == 0:
            text += '/* There are no published services.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('CALLED SERVICE FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_callservice_funcs */\n/** @{ */\n\n'
        
        for name in sorted(self.callServices):
            text += banner_small('CALLED SERVICE: ' + name) + '\n\n'
            text += '/** @name Service <tt>%s</tt> caller */\n/** @{ */\n\n' % name
            text += self.gen_callservice_handler(name) + '\n\n'
            text += '/** @} */\n\n'
        if len(self.callServices) == 0:
            text += '/* There are no called services.*/\n\n'
        
        text += '/** @} */\n\n'
        text += banner_big('GLOBAL FUNCTIONS') + '\n\n'
        text += '/** @addtogroup tcpros_funcs */\n/** @{ */\n\n'
        text += self.gen_regpubtopics_func() + '\n\n'
        text += self.gen_unregpubtopics_func() + '\n\n'
        text += self.gen_regsubtopics_func() + '\n\n'
        text += self.gen_unregsubtopics_func() + '\n\n'
        text += self.gen_regpubservices_func() + '\n\n'
        text += self.gen_unregpubservices_func() + '\n\n'
        text += '/** @} */\n\n'
        return text
    
    def handlers_source_path(self):
        path = self.cfgPath + os.sep + self.opts['sourceDir'];
        return os.path.normpath(path + os.sep + self.opts['handlersFilename'] + '.c')
        
    def export_handlers_source(self, text):
        with open(self.handlers_source_path(), 'w') as f:
            f.write(text)

###############################################################################

def print_usage():
    print 'Usage:\t%s <config_file>\n' % os.path.basename(sys.argv[0])
    print 'If <config_file> is ".", it is read from the standard input.'
    print 'Default configuration:'
    gen = CodeGen()
    print '\n[Options]'
    for k in sorted(gen.opts):
        print '%s = %s' % (k, gen.opts[k])
    print '\n[PubTopics]\n'
    print '[SubTopics]\n'
    print '[PubServices]\n'
    sys.stdout.flush()

def main():
    if len(sys.argv) != 2:
        print_usage()
        exit()
    
    cfgpath = sys.argv[-1]
    
    gen = CodeGen()
    
    print 'Configuration file [%s] ...' % ('<stdin>' if cfgpath == '.' else cfgpath),
    gen.load(cfgpath)
    print 'done'
    sys.stdout.flush()
    
    print 'Configuration contents:'
    print '\n[Options]'
    for k in sorted(gen.opts):
        print '%s = %s' % (k, gen.opts[k])
    print '\n[PubTopics]'
    for name in gen.pubTopics:
        print '%s = %s' % (name, gen.pubTopics[name]) 
    print '\n[SubTopics]'
    for name in gen.subTopics:
        print '%s = %s' % (name, gen.subTopics[name])
    print '\n[PubServices]'
    for name in gen.pubServices:
        print '%s = %s' % (name, gen.pubServices[name])
    print '\n[CallServices]'
    for name in gen.callServices:
        print '%s = %s' % (name, gen.callServices[name])
    print '\n# EOF\n'
    sys.stdout.flush()

    print 'Retrieving data types from ROS ...',
    sys.stdout.flush()
    gen.elaborate()
    print 'done'
    print 'Message types sorted by dependency:'
    for name in gen.sortedMsgTypeNames:
        print '\t' + name
    sys.stdout.flush()

    print ''    
    if str2bool(gen.opts['genMsgTypesHeader']):
        print 'Types header file [%s] ...' % gen.msgtypes_header_path(),
        msgTypesHeader = gen.gen_msgtypes_header()
        gen.export_msgtypes_header(msgTypesHeader)
        print 'done'
        sys.stdout.flush()
    
    if str2bool(gen.opts['genMsgTypesSource']):
        print 'Types source file [%s] ...' % gen.msgtypes_source_path(),
        msgTypesSource = gen.gen_msgtypes_source()
        gen.export_msgtypes_source(msgTypesSource)
        print 'done'
        sys.stdout.flush()
    
    if str2bool(gen.opts['genHandlersHeader']):
        print 'Handlers header file [%s] ...' % gen.handlers_header_path(),
        handlersHeader = gen.gen_handlers_header()
        gen.export_handlers_header(handlersHeader)
        print 'done'
        sys.stdout.flush()
    
    if str2bool(gen.opts['genHandlersSource']):
        print 'Handlers source file [%s] ...' % gen.handlers_source_path(),
        handlersSource = gen.gen_handlers_source()
        gen.export_handlers_source(handlersSource)
        print 'done'
        sys.stdout.flush()

main()
