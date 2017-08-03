import patParser
import inspect
import os
import time
import re

_SKIP_LONG_TEST = True

def rmIndent( s ):
    return re.sub( "\n +", "\n", s )



class Report(object):
    def __init__( self, content, path, lineno ):
        self.path    = path
        self.lineno  = lineno
        self.content = content
    
    def __repr__( self ):
        los = []
        for eline in self.content.splitlines():
            los.append("    %s"%eline)
        
        return "Line %d: %s"%( self.lineno, "\n".join( los ) )



class Reports(object):
    def __init__( self ):
        self.container = {}
    
    def add( self, typ, additional='' ):
        fobj, path, lineno, funcname, codelines, v = inspect.stack()[2]
        
        content = "\n".join( codelines ) + "\n"
        if len(str(additional)) > 0:
            content += "<Bad data> " + str( additional ) + "\n"
        
        if typ not in self.container:
            self.container.update( { typ : [] } )
        self.container[typ].append( Report( content, path, lineno ) )
    
    def __repr__( self ):
        los = []
        for typ, reps in self.container.iteritems():
            los.append( 'Type "%s":'%( typ ))
            for rep in reps:
                los.append( "    %s"%( rep ))
        return "\n".join( los )
            
            

class T(object):
    def __init__( self ):
        self.reports = Reports()
        self.testsAmounts = {}
        fobj, path, lineno, funcname, codelines, v = inspect.stack()[1]
        self.parentName = funcname
    
    def _isError( self, f, *argv, **args): 
        try:
            f(*argv, **args)
        except:
            return 1
        return 0
    
    def _count( self ):
        # stack:
        # [ frame object, filePath, lineno, thisfunctionName, listOfCodeLine, 0 ]
        # [] of previouse one
        # [] of previouse previouse one
        # ...
        fobj, path, lineno, funcname, codelines, v = inspect.stack()[1]
        if funcname not in self.testsAmounts:
            self.testsAmounts.update( { funcname : 0 })
        self.testsAmounts[ funcname ] += 1
    
    def beExcept( self, f, *argv, **args ):
        self._count()
        if not self._isError( f, *argv, **args):
            self.reports.add( "Missing exception" )
    
    def bePass( self, f, *argv, **args):
        self._count()
        if self._isError( f, *argv, **args):
            self.reports.add( "Unwanted excpetion" )
    
    def beEq( self, v, c ):
        self._count()
        if v != c:
            self.reports.add( "Equal test failed", v )
    
    def beNeq( self, v, c ):
        if v == c:
            self.reports.add( "Non-equal test failed", v )
    
    def beTrue( self, b ):
        if not b:
            self.reports.add( "Ture test failed", b )
    
    def _mkHeadline( self, s ):
        preLen = max( ((80-len(s))/2), 0 )
        postLen = preLen
        return preLen, "_"*preLen + s + "_"*postLen
    
    def _getStatistics( self ):
        preLen, headline = self._mkHeadline( '"%s" all passed'%self.parentName )
        
        los = [ headline ]
        indent = " "*preLen
        
        for funcname, count in self.testsAmounts.iteritems():
            los.append( "%s%s: %d"%( indent, funcname, count ))
        los.append("")
        return '\n'.join( los )
    
    def __repr__( self ):
        log = str(self.reports)
        
        if len( log ) == 0:
            return self._getStatistics()
        else:
            preLen, headline = self._mkHeadline( '"%s" failure reports'%self.parentName )
            return '%s\n\n%s\n'%( headline, log )



def t_parseTS():
    t = T()
    
    #-----------------------------------------------------------Bad format-----
    t.beExcept( patParser.parseTS, 'slot 2,4,6,8,64' )
    t.beExcept( patParser.parseTS, 'slots: 3' )
    t.beExcept( patParser.parseTS, 'slot: 0' )
    
    #----------------------------------------------------Corner conditions-----
    t.bePass( patParser.parseTS, ' SlOt\t: 1,\t65 // Hello Slot:0' )
    
    #--------------------------------------------------------------Compare-----
    settings, slotTimes, slots = patParser.parseTS( """
        ts0 = 100
        slot:1
        ts:0 ch:1
        {
            0 = 16h
        }
    """ )
    
    expectLines = ["PEB_SLOT,TIM_SET,PEB_CH,TS_CH,D0 LOC,D0 DAT,D1 LOC,D1 DAT,C0 LOC,C1 LOC,C DAT,C MOD,T0 LOC,T0 DAT,T1 LOC,T1 DAT"]
    expectLines.append( "1,0,1,0,0,0,0,0,10,0,1,0,0,3,0,3" )
    
    expectTxt = "\n".join( expectLines )
    t.beEq( settings.exportEntireCsv(), expectTxt )
    
    
    settings, slotTimes, slots = patParser.parseTS( """
        ts0 = 100
        slot:1
        ts:0 ch:1
        {
            0 = 16-17l
        }
    """ )
    
    expectLines = ["PEB_SLOT,TIM_SET,PEB_CH,TS_CH,D0 LOC,D0 DAT,D1 LOC,D1 DAT,C0 LOC,C1 LOC,C DAT,C MOD,T0 LOC,T0 DAT,T1 LOC,T1 DAT"]
    expectLines.append( "1,0,1,0,0,0,0,0,10,11,0,1,0,3,0,3" )
    
    expectTxt = "\n".join( expectLines )
    t.beEq( settings.exportEntireCsv(), expectTxt )
    
    #-----------------------------------------------------Complete Example-----
    def t_commcase():
        settings, slotTimes, slots = patParser.parseTS( """
        ts0 = 1us
        ts2 = 2us
        slot:1,3
        ts:0,1 ch:1,2,3
        {
            0 = 0o 0u     // set 0: drive high from the start
            1 = 0o 0u 10d // set 1: drive a 10ns pulse from the start
            2 = 0i 10L    // set 2: become an input, then test if the voltage at 10ns is low
            3 = 0i 10-20L // set 3: become an input, then test if the voltage keeps low in the window of 10ns to 20ns
            4 = 1uso 1msu // set 4: drive high at 1us, then low at 1ms
            5 = 0o 0u 10i 11-15m // set 5: drive a high pulse at the begining, then becomes an input, wait 1ns, then check if the voltage keeps in the range from VOL to VOH for 4ns
        }
        """ )
        t.beEq( slots, set([1,3]) )
        t.beEq( slotTimes, { 0: 1000.0, 2: 2000.0 } )
        
        stts = [
              "0,0,1,0,0,0,0,3,0,0,1,0,3"
            , "1,0,1,A,0,0,0,3,0,0,1,0,3"
            , "2,0,0,0,0,A,0,0,0,0,0,0,3"
            , "3,0,0,0,0,A,14,0,1,0,3,0,3"
            , "4,1,1,F4240,1,0,0,3,0,0,3,0,3"
            , "5,0,0,0,0,B,F,2,1,0,3,0,3"
        ]
        expectLines = ["PEB_SLOT,TIM_SET,PEB_CH,TS_CH,D0 LOC,D0 DAT,D1 LOC,D1 DAT,C0 LOC,C1 LOC,C DAT,C MOD,T0 LOC,T0 DAT,T1 LOC,T1 DAT"]
        for slot in (1,3):
            for ts in (0,1):
                for ch in (1,2,3):
                    for st in stts:
                        expectLines.append( "%d,%d,%d,%s"%(slot, ts, ch, st))
        expectTxt = "\n".join( expectLines )
        t.beEq( settings.exportEntireCsv(), expectTxt)
    
    t_commcase()
    #----------------------------------------------------Extreme Condition-----
    def t_megadata():
        tstart = time.clock()
        settings, slotTimes, slots = patParser.parseTS( """
        ts0 = 1us
        ts1 = 1us
        ts2 = 2us
        ts4 = 2us
        ts5 = 2us
        ts6 = 2us
        ts7 = 2us
        ts8 = 2us
        slot:1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
        ts:0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 ch:1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64
        {
            0 = 0o 0u     // set 0: drive high from the start
            1 = 0o 0u 10d // set 1: drive a 10ns pulse from the start
            2 = 0i 10L    // set 2: become an input, then test if the voltage at 10ns is low
            3 = 0i 10-20L // set 3: become an input, then test if the voltage keeps low in the window of 10ns to 20ns
            4 = 1uso 1msu // set 4: drive high at 1us, then low at 1ms
            5 = 0o 0u 10i 11-15m // set 5: drive a high pulse at the begining, then becomes an input, wait 1ns, then check if the voltage keeps in the range from VOL to VOH for 4ns
            6 = 0o 0u 10i 11-15m // set 5: drive a high pulse at the begining, then becomes an input, wait 1ns, then check if the voltage keeps in the range from VOL to VOH for 4ns
            7 = 0o 0u 10i 11-15m // set 5: drive a high pulse at the begining, then becomes an input, wait 1ns, then check if the voltage keeps in the range from VOL to VOH for 4ns
        }
        """ )
        
        csvlen = len(settings.exportEntireCsv())
        tstop = time.clock()
        
        t.beEq( slots, set(range(1,17)) )
        # t.beEq( slotTimes, { 0: 1000.0, 2: 2000.0 } )
        
        t.beTrue( tstop - tstart < 10 ) # time cost
        t.beEq( csvlen, 17932389 ) # csv length
    
    if not _SKIP_LONG_TEST:
        t_megadata()
    
    #--------------------------------------------------------Serial Number-----
    settings, slotTimes, slots = patParser.parseTS( """
    ts0 = 1us
    slot:1-2,3
    ts:4-5 ch:6-7
    {
        0 = 0o 0u     // set 0: drive high from the start
    }
    """ )
    t.beEq(
        settings.exportEntireCsv()
        , rmIndent( """PEB_SLOT,TIM_SET,PEB_CH,TS_CH,D0 LOC,D0 DAT,D1 LOC,D1 DAT,C0 LOC,C1 LOC,C DAT,C MOD,T0 LOC,T0 DAT,T1 LOC,T1 DAT
                       1,4,6,0,0,1,0,0,0,0,3,0,0,1,0,3
                       1,4,7,0,0,1,0,0,0,0,3,0,0,1,0,3
                       1,5,6,0,0,1,0,0,0,0,3,0,0,1,0,3
                       1,5,7,0,0,1,0,0,0,0,3,0,0,1,0,3
                       2,4,6,0,0,1,0,0,0,0,3,0,0,1,0,3
                       2,4,7,0,0,1,0,0,0,0,3,0,0,1,0,3
                       2,5,6,0,0,1,0,0,0,0,3,0,0,1,0,3
                       2,5,7,0,0,1,0,0,0,0,3,0,0,1,0,3
                       3,4,6,0,0,1,0,0,0,0,3,0,0,1,0,3
                       3,4,7,0,0,1,0,0,0,0,3,0,0,1,0,3
                       3,5,6,0,0,1,0,0,0,0,3,0,0,1,0,3
                       3,5,7,0,0,1,0,0,0,0,3,0,0,1,0,3"""))
    
    
    t.beEq(slots, set([1,2,3]))
    
    
    print t
    
    
def t_parseTH():
    t = T()
    
    #-------------------------------------------------------Negitive value-----
    t.beEq(
        patParser.parseTH(
            """
            slot: 4 ch: 1
            {
                vih = -1.5
                vil = -1
            }
            """)
            
            ,
            
            {(4, 1): {'vih': -1.5, 'vil': -1.0}}
        )
    
    t.beEq(
        patParser.parseTH(
            """
            slot: 4 ch: 1
            {
                vih = -1.5
                vil = -1
            }
            """).exportEntireCsv()
            
            ,
            
            "4,1,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X"
        )
    
    #--------------------------------------------------------Serial Number-----
    t.beEq(
        patParser.parseTH(
            """
            slot: 4-6 ch: 2-3
            {
                vih = -1.5
                vil = -1
            }
            """)
            
            ,
            
            {
                 (4, 2): {'vih': -1.5, 'vil': -1.0}
               , (4, 3): {'vih': -1.5, 'vil': -1.0}
               , (5, 2): {'vih': -1.5, 'vil': -1.0}
               , (5, 3): {'vih': -1.5, 'vil': -1.0}
               , (6, 2): {'vih': -1.5, 'vil': -1.0}
               , (6, 3): {'vih': -1.5, 'vil': -1.0}
            }
        )
    
    t.beEq(
        patParser.parseTH(
            """
            slot: 4-6,8 ch: 2-3
            {
                vih = -1.5
                vil = -1
            }
            """).exportEntireCsv()
            
            ,
            
            rmIndent(
            """4,2,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               4,3,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               5,2,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               5,3,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               6,2,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               6,3,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               8,2,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X
               8,3,-1.5,-1.0,X,X,X,X,X,X,X,X,X,X""")
        )
    
    #-------------------------------------------------------Boundary Check-----
    t.beExcept(
        patParser.parseTH
        ,
        """
        slot: 1 ch: 0
        {
            vih = 0
        }
        """
    )
    t.beExcept(
        patParser.parseTH
        ,
        """
        slot: 0 ch: 1
        {
            vih = 0
        }
        """
    )
    
    print t


def t_parseINS():
    t = T()
    
    #----------------------------------------------------------Simple Case-----
    t.bePass( patParser.parseINS, "[1] 1\n  @1-3\n  NOP 0 0 s1c1=1" )
    
    #---------------------------------------------------------Missing slot-----
    t.beExcept( patParser.parseINS, "[1] 1\nNOP 0 0 s1c1=1" )
    
    #----------------------------------------Missing channel set nicknames-----
    t.beExcept( patParser.parseINS, "@1,3\nNOP 0 0 s1c1=1" )
    
    #-------------------------------------------Slot Number Boundary Check-----
    t.beExcept( patParser.parseINS, "[1] 1\n  @0\n  NOP 0 0 s1c1=1" )
    t.beExcept( patParser.parseINS, "[1] 1\n  @1\n  NOP 0 0 s0c1=1" )
    t.beExcept( patParser.parseINS, "[1] 1\n  @1\n  NOP 0 0 s1c0=1" )
    t.beExcept( patParser.parseINS, "[1] 1\n  @1\n  NOP 0 0 s1c0-2=1" )
    t.beExcept( patParser.parseINS, "[1] 1\n  @1\n  NOP 0 0 s1c2-0=1" )
    
    #--------------------------------------------------Range Boundary Swap-----
    t.bePass( patParser.parseINS, "[1] 1\n  @1\n  NOP 0 0 s1c2-1=1" )
    
    insMem, patMem = patParser.parseINS( "[1] 1\n  @1\n  NOP 0 0 s1c2-1=1" )
    
    t.beEq( insMem.exportEntireCsv(), "1,0,0" )
    t.beEq( patMem.exportEntireCsv(), "1,0,FFFFFFFFFFC9,FFFFFFFFFFFF,FFFFFFFFFFFF,FFFFFFFFFFFF" )
    
    #--------------------------------------Serial Format On Channel Number-----
    t.bePass( patParser.parseINS, "[1] 1\n  @1-3\n  NOP 0 0 s1c1-4=1" )
    
    #--------------------------------------------Check Data In Simple Case-----
    insMem, patMem = patParser.parseINS( "[1] 1\n  @1-3,5\n  NOP 0 0x8 s1c1-4=1" )
    
    t.beEq( insMem.exportEntireCsv(), rmIndent("""1,0,8
                                                  2,0,8
                                                  3,0,8
                                                  5,0,8"""))
    t.beEq( patMem.exportEntireCsv(), "1,0,FFFFFFFFF249,FFFFFFFFFFFF,FFFFFFFFFFFF,FFFFFFFFFFFF" )
    
    #-------------------------------------------------------Default Number-----
    insMem, patMem = patParser.parseINS( "default 0\n  [1] 1\n  @1-3,5\n  NOP 0 0x8 s1c1-4=1" )
    
    t.beEq( insMem.exportEntireCsv(), rmIndent("""1,0,8
                                                  2,0,8
                                                  3,0,8
                                                  5,0,8"""))
    t.beEq( patMem.exportEntireCsv(), "1,0,249,0,0,0" )
    
    #-----------------------------------------------------------Multilines-----
    insMem, patMem = patParser.parseINS( "default 0\n  [1] 1\n  [2] Z\n  @1-3,5\n  NOP 0 0x8 s1c1-4=1 s2c2=Z\n  NOP 0 0x3 s1c1-4=Z s2c2=1\n  " )
    
    t.beEq( insMem.exportEntireCsv(), rmIndent("""1,0,8
                                                  1,1,3
                                                  2,0,8
                                                  2,1,3
                                                  3,0,8
                                                  3,1,3
                                                  5,0,8
                                                  5,1,3"""))
    t.beEq(
            patMem.exportEntireCsv()
        ,
            rmIndent(
            """1,0,249,0,0,0
            1,1,492,0,0,0
            2,0,10,0,0,0
            2,1,8,0,0,0""")
        )
    
    #-------------------------------------------------------Mega Data Test-----
    def t_megadata():
        codes = ["default 0"]
        codes.append("[1] 1")
        codes.append("[2] Z")
        codes.append("@1-16")
        allChStr = " ".join([ "s%dc1-64=1"%s for s in xrange(1,64+1) ])
        allChCode = "NOP 0 0xFF %s"%allChStr
        
        for i in xrange( 10000 ):
            codes.append( allChCode )
        
        insMem, patMem = patParser.parseINS( "\n".join( codes ))
        
        t.beEq( len( patMem ), 10000*64 )
        t.beEq( len( insMem ), 10000*16 )
        t.beEq( hash( patMem.exportEntireCsv() ), -2009098329 )
        t.beEq( hash( insMem.exportEntireCsv() ), -536281617 )
    
    if not _SKIP_LONG_TEST: t_megadata()
    
    print t


def t_parseSITE():
    t = T()
    
    #----------------------------------------------------------Single Link-----
    linksCsv = patParser.parseSITE('').exportLinksCsv()
    t.beEq( linksCsv, "" )
    
    linksCsv = patParser.parseSITE('1=s1c1').exportLinksCsv()
    t.beEq( linksCsv, "1,1,1,0" )
    
    linksCsv = patParser.parseSITE('1=s1c1-2').exportLinksCsv()
    t.beEq( linksCsv, "1,1,3,0" )
    
    linksCsv = patParser.parseSITE('1=s1c2-16').exportLinksCsv()
    t.beEq( linksCsv, "1,1,FFFE,0" )
    
    linksCsv = patParser.parseSITE('1=s1-2 c2-16').exportLinksCsv()
    t.beEq( linksCsv, "1,1,FFFE,0\n2,1,FFFE,0" )
    
    #--------------------------------------------------------------2 Links-----
    linksCsv = patParser.parseSITE('1=s1c1-4\n2=s1c1-2').exportLinksCsv()
    t.beEq( linksCsv, "1,1,F,3" )
    
    #--------------------------------------------------------------3 Links-----
    t.beExcept( patParser.parseSITE, '1=s1c1-4\n2=s1c1-2\n3=s1c3-4' )
    
    #----------------------------------------------------------------Sites-----
    sitesCsv = patParser.parseSITE('').exportSitesCsv()
    t.beEq( sitesCsv, "" )
    
    sitesCsv = patParser.parseSITE('1=s1c1').exportSitesCsv()
    t.beEq( sitesCsv, "1,1,1" )
    
    sitesCsv = patParser.parseSITE('1=s1c16-17').exportSitesCsv()
    t.beEq( sitesCsv, "1,1,5" )
    
    sitesCsv = patParser.parseSITE('1=s1c16-49').exportSitesCsv()
    t.beEq( sitesCsv, "1,1,55" )
    
    #--------------------------------------------------Sites on Both Links-----
    sitesCsv = patParser.parseSITE('1=s1c1-8\n2=s1c9-16').exportSitesCsv()
    t.beEq( sitesCsv, "1,1,1\n1,2,2" )
    
    #--------------------------------------------------Sites on Both Links-----
    sitesCsv = patParser.parseSITE('''
    1= s1c1-10 s2c1-10
    2= s1c11-30
    3= s1c31-64
    ''').exportSitesCsv()
    t.beEq( sitesCsv, "1,1,1\n1,2,6\n1,3,58\n2,1,1" )
    
    
    
    
    print t


def strToInt( s ):
    if s.upper().startswith("0X"):
        try:
            return int( s, 16 )
        except:
            raise Exception('Illegal hex "%s"'%s)
    else:
        try:
            return int( s, 10 )
        except:
            raise Exception('Illegal digit "%s"'%s)

def t_parseLOG():
    t = T()
    
    t.beEq( patParser.parseLOG("@7\nlog 10,0,n,0"),   "7,A,0,0,0")
    t.beEq( patParser.parseLOG("@7\nlog 0x10,0,n,0"), "7,10,0,0,0")
    t.beEq( patParser.parseLOG("@7\nlog 0,11,n,0"),   "7,0,B,0,0")
    t.beEq( patParser.parseLOG("@7\nlog 0,11,n,1"),   "7,0,B,0,1")
    t.beEq( patParser.parseLOG("@7\nlog 0,0,f,0"),    "7,0,0,2,0")
    t.beEq( patParser.parseLOG("@7\nlog 0,0,r,0"),    "7,0,0,3,0")
    t.beEq( patParser.parseLOG("@7\nlog 0,0,R,0"),    "7,0,0,3,0")
    t.beEq( patParser.parseLOG("@7\n\tLog  0 , 0 , R , 0 "),    "7,0,0,3,0")
    t.beEq( patParser.parseLOG("@7\n"),    "7,0,0,0,0")
    t.beEq( patParser.parseLOG("@7//log 3,4,R,0"),    "7,0,0,0,0")
    
    t.beEq( patParser.parseLOG("@7-8\nlog 10,0,n,0"),   "7,A,0,0,0\n8,A,0,0,0")
    t.beEq( patParser.parseLOG("log 10,0,n,0\n@8\n@7"),   "7,A,0,0,0\n8,A,0,0,0")
    
    t.beExcept( patParser.parseLOG, "" )
    t.beExcept( patParser.parseLOG, "0,0,n,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog ,0,r,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog 0,,r,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog 0,0,x,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog 0,0,ff,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog 0,0,n,3" )
    t.beExcept( patParser.parseLOG, "@7\nlog -1,0,n,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog 0,-1,n,0" )
    t.beExcept( patParser.parseLOG, "@7\nlog 1.1,0,n,0" )
    print t
    
    

if __name__ == "__main__":
    # t_parseTS()
    # t_parseTH()
    # t_parseINS()
    # t_parseSITE()
    t_parseLOG()
    raw_input("Press ENTER to close window...\n")