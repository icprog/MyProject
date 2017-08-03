# coding:utf-8
import sys
import re
import traceback
import linecache
import collections
from cStringIO import StringIO

# _DEBUG = 0
_DEBUG = True

# make print string to a buffer
old_stdout = sys.stdout
logout = StringIO()


# Utilities
def clearCmt( txt, mark='//' ):
    los = []
    for eline in txt.splitlines():
        p = eline.find(mark)
        los.append( eline[:p] if p >=0 else eline )
    return '\n'.join(los)


#--- TIME SLOTS -------------------------------------------------------
def timeUnitTons( unitStr ):
    uStr = unitStr.lower()
    if uStr == "": return 1
    if uStr == "ns": return 1
    if uStr == "us": return 1000
    if uStr == "ms": return 1000000
    if uStr == "s" : return 1000000000
    if uStr == "sec": return 1000000000
    raise Exception('Illegal time unit "%s"'%unitStr)


def toUnitTime( num, timeUnitString ):
    return float( num ) * timeUnitTons( timeUnitString )


class Action(object):
    def __init__(self, name, dat, loc):
        self.name = name
        self.dat = dat
        self.loc = loc
    
    def listDataChars( self ):
        # override
        return []
    
    def getDataCharsInfos( self ):
        # override
        return {}


class CompAction(Action):
    MODE_STROBE = 0
    MODE_WINDOW = 1
    MODE_DONT_CARE = 2
    
    DATA_LOW = 0
    DATA_HIGH = 1
    DATA_MIDDLE = 2
    DATA_VALID = 3
    DATA_DONT_CARE = 4
    def __init__(self, name ):
        dat = self.DATA_DONT_CARE
        loc = 0
        loc2 = 0
        
        Action.__init__( self, name, dat, loc )
        
        self.loc2 = loc2
        self.mode = self.MODE_STROBE
        
        self._lvMapDat = {
              'h' : self.DATA_HIGH
            , 'l' : self.DATA_LOW
            , 'm' : self.DATA_MIDDLE
            , 'v' : self.DATA_VALID
            , 'x' : self.DATA_DONT_CARE
        }
    
    def setWindowMode( self, loc, loc2, lv ):
        self.loc = int( min( loc, loc2 ))
        self.loc2 = int( max( loc, loc2 ))
        self.dat = self._lvToDat( lv )
        if self.dat == self.DATA_DONT_CARE:
            self.mode = self.MODE_DONT_CARE
            self.dat  = self.DATA_LOW
        else:
            self.mode = self.MODE_WINDOW
    
    def setStrobeMode( self, loc, lv ):
        self.loc = int( loc )
        self.dat = self._lvToDat(lv)
        if self.dat == self.DATA_DONT_CARE:
            self.mode = self.MODE_DONT_CARE
            self.dat  = self.DATA_LOW
        else:
            self.mode = self.MODE_STROBE
    
    def _lvToDat( self, lv ):
        dat = self._lvMapDat.get( lv.lower() )
        if dat is None:
            raise Exception( 'Illegal level string "%s", should be one of the %s '%( ",".join( self._lvMapDat.keys() )))
        return dat
    
    def listDataChars( self ):
        return self._lvMapDat.keys()
    
    def getDataCharsInfos( self ):
        infos = collections.OrderedDict()
        infos.update({ 'h' : "Compare high.  Test passes if the DUT's output voltage is higher than VOH of this channel" })
        infos.update({ 'l' : "Compare low.  Test passes if the DUT's output voltage is lower than VOL of this channel" })
        infos.update({ 'm' : "Compare middle. Test passes if the DUT's output voltage is between VOL and VOH of this channel" })
        infos.update({ 'x' : "Don't care the comparing result.  Test will pass anyway" })
        
        if set(self.listDataChars()) != set(infos.keys()):
            raise Exception( "IOAction's help content does not match the code, please update it" )
        
        return infos
    

class DriveAction(Action):
    DATA_LOW = 0
    DATA_HIGH = 1
    DATA_FALL = 2
    DATA_RISE = 3
    def __init__(self, name ):
        dat = self.DATA_LOW
        loc = 0
        
        Action.__init__( self, name, dat, loc )
        
        self._lvMapDat = {
              'u' : self.DATA_HIGH
            , 'd' : self.DATA_LOW
            , 'r' : self.DATA_RISE
            , 'f' : self.DATA_FALL
        }
    
    def setLevel( self, loc, lv ):
        self.loc = int(loc)
        self.dat = self._lvToDat( lv )
    
    def _lvToDat( self, lv ):
        dat = self._lvMapDat.get( lv.lower() )
        if dat is None:
            raise Exception( 'Illegal level string "%s", should be one of the %s '%( ",".join( self._lvMapDat.keys() )))
        return dat
    
    def listDataChars( self ):
        return self._lvMapDat.keys()
    
    def getDataCharsInfos( self ):
        infos = collections.OrderedDict()
        infos.update({ 'u' : "Drive the channel high, 'u' denotes \"up\"" })
        infos.update({ 'd' : "Drive the channel low, 'd' denotes \"down\"" })
        infos.update({ 'r' : "The output voltage went low at first, then drive high to form a rising edge.  'r' denotes \"rising\"" })
        infos.update({ 'f' : "The output voltage went high at first, then drive low to form a falling edge.  'f' denotes \"falling\"" })
        
        if set(self.listDataChars()) != set(infos.keys()):
            raise Exception( "IOAction's help content does not match the code, please update it" )
        return infos


class IOAction(Action):
    DATA_HZ = 0
    DATA_DRIVE = 1
    DATA_DONT_CHANGE = 3
    def __init__( self, name ):
        dat = self.DATA_DONT_CHANGE
        loc = 0
        Action.__init__( self, name, dat, loc )
        
        self._dirMapDat = {
              'i' : self.DATA_HZ
            , 'o' : self.DATA_DRIVE
            , 'p' : self.DATA_DONT_CHANGE
        }
    
    def setIO( self, loc, direction ):
        self.loc = int(loc)
        self.dat = self._dirToDat( direction )
    
    def _dirToDat( self, direction ):
        dat = self._dirMapDat.get( direction.lower() )
        if dat is None:
            raise Exception( 'Illegal direction string "%s", should be one of the %s '%( ",".join( self._dirMapDat.keys() )))
        return dat
    
    def listDataChars( self ):
        return self._dirMapDat.keys()
    
    def getDataCharsInfos( self ):
        infos = collections.OrderedDict()
        infos.update({ 'i' : "Change this channel into an input on the fly" })
        infos.update({ 'o' : "Change this channel into an output on the fly" })
        infos.update({ 'p' : "Keep this channel's direction as is was" })
        
        if set(self.listDataChars()) != set(infos.keys()):
            raise Exception( "IOAction's help content does not match the code, please update it" )
        return infos


class ChannelSetting(object):
    def __init__( self, name="" ):
        self.name = name
        self._drv0 = DriveAction( "d0" )
        self._drv1 = DriveAction( "d1" )
        self._cmp = CompAction( "c" )
        self._io0 = IOAction( "t0" )
        self._io1 = IOAction( "t1" )
        
        self._drv0done = False
        self._drv1done = False
        self._cmpdone = False
        self._io0done = False
        self._io1done = False
        
        self._checkActionCharsConflict()
    
    def exportCsvColumn( self ):
        return "D0 LOC,D0 DAT,D1 LOC,D1 DAT,C0 LOC,C1 LOC,C DAT,C MOD,T0 LOC,T0 DAT,T1 LOC,T1 DAT"
    
    def exportCsv( self ):
        hexes = []
        seqs = [
              self._drv0.loc
            , self._drv0.dat
            , self._drv1.loc
            , self._drv1.dat
            , self._cmp.loc
            , self._cmp.loc2
            , self._cmp.dat
            , self._cmp.mode
            , self._io0.loc
            , self._io0.dat
            , self._io1.loc
            , self._io1.dat
        ]
        
        hexes = [ "%X"%(v) for v in seqs ]
        return ",".join( hexes )
    
    def listDriverChars( self ):
        return DriveAction('').listDataChars()
    
    def listIOChars( self ):
        return IOAction('').listDataChars()
    
    def listCompareChars( self ):
        return CompAction('').listDataChars()
    
    def _checkActionCharsConflict( self ):
        dr = DriveAction('').listDataChars()
        cm = CompAction('').listDataChars()
        io = IOAction('').listDataChars()
        if len( set( dr + cm + io )) != len( dr ) + len( cm ) + len( io ):
            raise Exception("A BUG WAS FOUND! Action characters conflicts, please email aaron_chang@tri.com.tw")
    
    def setWindowCompare( self, loc, loc2, lv ):
        if self._cmpdone: raise Exception("Trying to re-setup compare on channel %s"%(self.name))
        self._cmpdone = True
        self._cmp.setWindowMode( loc, loc2, lv )
    
    def setStrobeCompare( self, loc, lv ):
        if self._cmpdone: raise Exception("Trying to re-setup compare on channel %s"%(self.name))
        self._cmpdone = True
        self._cmp.setStrobeMode( loc, lv )
    
    def newDriver( self, loc, lv ):
        if not self._drv0done:
            self._drv0done = True
            self._drv0.setLevel( loc, lv )
        elif not self._drv1done:
            self._drv1done = True
            self._drv1.setLevel( loc, lv )
        else:
            raise Exception("Falied to set the third driver marker, only two supported on channel %s"%(self.name))
    
    def newIO( self, loc, direction ):
        if not self._io0done:
            self._io0done = True
            self._io0.setIO( loc, direction )
        elif not self._io1done:
            self._io1done = True
            self._io1.setIO( loc, direction )
        else:
            raise Exception("Falied to set the third io marker, only two supported on channel %s"%(self.name))

def FormatException():
    exc_type, exc_obj, tb = sys.exc_info()
    f = tb.tb_frame
    lineno = tb.tb_lineno
    filename = f.f_code.co_filename
    linecache.checkcache(filename)
    line = linecache.getline(filename, lineno, f.f_globals)
    return 'EXCEPTION IN ( {}, LINE {} )'.format(filename, lineno)
    
def PrintException():
    exc_type, exc_obj, tb = sys.exc_info()
    f = tb.tb_frame
    lineno = tb.tb_lineno
    filename = f.f_code.co_filename
    linecache.checkcache(filename)
    line = linecache.getline(filename, lineno, f.f_globals)
    print 'EXCEPTION IN ( {}, LINE {} )'.format(filename, lineno)


class TimeslotSettings( collections.OrderedDict ):
    def __init__( self, *argv, **args ):
        collections.OrderedDict.__init__( self, *argv, **args )
    
    def setPebSlotUsed( self, slots ):
        self.slots = slots
    
    def createTimeslot( self, name ):
        if name not in self:
            self.update( { name : collections.OrderedDict() })
    
    def createChannel( self, ts, pebChnum, name ):
        self.createTimeslot( ts )
        if pebChnum not in self[ts]: self[ts].update({ pebChnum : collections.OrderedDict() })
        self[ts][pebChnum].update( { name : ChannelSetting( name ) })
    
    def setWindowCompare( self, ts, pebChnum, ch, loc, loc2, lv ):
        self[ts][pebChnum][ch].setWindowCompare( loc, loc2, lv )
        
    def setStrobeCompare( self, ts, pebChnum, ch, loc, lv ):
        self[ts][pebChnum][ch].setStrobeCompare( loc, lv )
    
    def newDriver( self, ts, pebChnum, ch, loc, lv ):
        self[ts][pebChnum][ch].newDriver( loc, lv )
    
    def newIO( self, ts, pebChnum, ch, loc, direction ):
        self[ts][pebChnum][ch].newIO( loc, direction )
    
    def exportEntireCsv( self ):
        csvs = []
        csvs.append( "PEB_SLOT,TIM_SET,PEB_CH,TS_CH,%s"%(ChannelSetting().exportCsvColumn() ))
        for slot in self.slots:
            for ts in self:
                for pebChnum in self[ts]:
                    for chname, chsettings in self[ts][pebChnum].iteritems():
                        csvs.append( "%X,%X,%X,%X,%s"%( slot, int(ts), int(pebChnum), int(chname), chsettings.exportCsv()))
        return "\n".join( csvs )


def helpTS():
    los = []
    
    los.append( "// Use this script to create time settings, and configure their 8 sets of markers for each channel" )
    los.append( "" )
    los.append( "Template:" )
    los.append( "" )
    los.append( "slot:<slot_nums>" )
    los.append( "ts<indicators> = <timing_set_in_decimal><time_unit>" )
    los.append( "ts:<indicators> ch: <channels>" )
    los.append( "{" )
    los.append( "    <set_number> = <location_in_decimal><time_unit><action_note>" )
    los.append( "}" )
    los.append( "<slot_nums>: Denotes the PEB slots used" )
    los.append( "<timing_set_in_decimal>: Denotes the period of this timing set" )
    los.append( "<indicators>: Denotes which timing set" )
    los.append( "<channels>: Denotes which channel" )
    los.append( "<location_in_decimal>: Denotes when the action will act" )
    los.append( '<time_unit>: Can be ""(default is ns), "us", "ms" or "sec"' )
    los.append( "<action_note>: Denotes the action maker" )
    los.append( "    Drive action marker: When the channel is an output, the markers change the drive level on the fly, there can be two drive action markers" )
    for c, info in DriveAction('').getDataCharsInfos().iteritems(): los.append( "        %s: %s"%( c, info ) )
    los.append( "    IO action marker: The markers change the channels direction level on the fly, there can be two IO action markers" )
    for c, info in IOAction('').getDataCharsInfos().iteritems(): los.append( "        %s: %s"%( c, info ) )
    los.append( "    Compare action marker: The markers change the test case on the fly, there are two modes, one is strobe mdoe and another is window mode" )
    for c, info in CompAction('').getDataCharsInfos().iteritems(): los.append( "        %s: %s"%( c, info ) )
    los.append( "" )
    los.append( "" )
    los.append( "Example: Configure time settings 0,1 on PEB channel 1,2,3 on slot 1 and 3" )
    los.append( "" )
    los.append( "slot:1,3" )
    los.append( "ts:0,1 ch:1,2,3" )
    los.append( "{" )
    los.append( "    0 = 0o 0u     // set 0: drive high from the start" )
    los.append( "    1 = 0o 0u 10d // set 1: drive a 10ns pulse from the start" )
    los.append( "    2 = 0i 10L    // set 2: become an input, then test if the voltage at 10ns is low" )
    los.append( "    3 = 0i 10-20L // set 3: become an input, then test if the voltage keeps low in the window of 10ns to 20ns" )
    los.append( "    4 = 1uso 1msu // set 4: drive high at 1us, then low at 1ms" )
    los.append( "    5 = 0o 0u 10i 11-15m // set 5: drive a high pulse at the begining, then becomes an input, wait 1ns, then check if the voltage keeps in the range from VOL to VOH for 4ns" )
    los.append( "}" )
    los.append( "" )
    
    return "\n\n" + "\n// ".join( los )

def numListLine2List( line, lowerTag=True ):
    ld = []
    for numStr in re.findall( "([\t 0-9,\-]+)", line, flags=re.IGNORECASE ):
        for gnumStr in numStr.split(','):
            gnumStr = gnumStr.strip()
            markAmount = gnumStr.count('-')
            
            if markAmount == 0:   ld.append( int( gnumStr ) )
            elif markAmount == 1:
                mrange = re.search( "(\d+)\s*-\s*(\d+)", gnumStr )
                if not mrange: raise
                a = int(mrange.group(1))
                b = int(mrange.group(2))
                for i in xrange( min(a,b), max(a,b)+1 ):
                    ld.append( int( i ) )
            else:
                raise
    return ld


def parseTS( txt ):
    # Support slot assignments
    
    parseFailed = False
    
    # Clear comments
    cleanlines = []
    for eline in txt.splitlines():
        p = eline.find("//")
        cleanlines.append( eline[:p] if p>=0 else eline  )
    content = '\n'.join(cleanlines)
    
    # Slot used
    slots = set([])
    for eline in content.splitlines():
        mslots = re.search( "^\s*slot\s*:\s*([\t 0-9,\-]+)\s*$", eline, flags=re.IGNORECASE )
        if mslots:
            for slotVal in numListLine2List(mslots.group(1)):
                if slotVal < 1:
                    raise Exception("Slot number started from 1")
                slots.add( slotVal )
    
    if len(slots) == 0:
        raise Exception("No slot assigned.  For example, please write \"slot:1,2,3\" in the TS file")
    
    
    # Time setting period
    
    settings = TimeslotSettings()
    settings.setPebSlotUsed( slots )
    los = []
    defineLine = {}
    slotTimes = {}
    slotTimeParser = re.compile( "^\s*ts(\d+)\s*=\s*(\d+)(\w*)", flags=re.IGNORECASE|re.MULTILINE )
    for mSlotTime in slotTimeParser.finditer( content ):
        tss, tms, tus = mSlotTime.groups()
        ts = int(tss)
        tm = toUnitTime( tms, tus )
        
        lineno = content.count("\n",0,mSlotTime.start()) + 1
        if ts in slotTimes:
            print "Line %d: Time slot %d re-defined (already defined in line %d)"%(lineno, ts, defineLine[ts])
            print "-"*80
            parseFailed = True
        
        defineLine.update( { ts : lineno } )
        slotTimes.update( { ts : tm })
    
    
    
    # 8 sets of markers for each channel
    
    blkParser = re.compile( "ts\s*:\s*([\t 0-9,\-]+)\s+ch\s*:\s*([\t 0-9,\-]+)\s*\{([^}]*)\}", flags=re.IGNORECASE )
    
    for mBlk in blkParser.finditer( content ):
        tssStr, pebChnumsStr, blkContent = mBlk.groups()
        
        
        # Expand all Time settings and PEB channel numbers collections from given
        collTsPebchnums = []
        # for tsStr in tssStr.split(','):
            # for pebChnum in pebChnumsStr.split(','):
                # collTsPebchnums.append([ tsStr.strip(), pebChnum.strip() ])
        for tsVal in numListLine2List( tssStr ):
            for pebChnumVal in numListLine2List( pebChnumsStr ):
                collTsPebchnums.append([ str(tsVal), str(pebChnumVal) ])
        
                
        
        # Parse the 8 sets of markers in time setting block
        for ts, pebChnum in collTsPebchnums:
            if int(pebChnum) <= 0 or int(pebChnum) > 64:
                raise Exception("Line %d: PEB CHANNEL \"%s\" out of range, should be 1~64"%( lineno, pebChnum ))
            
            settings.createTimeslot( ts )
            los.append(ts)
            sublineno = 0
            for sublineno, eline in enumerate(blkContent.splitlines()):
                try:
                    m = re.search( "(\d+)\s*=\s*([^=]+)", eline )
                    if not m: continue
                    ch, setup = m.group(1), m.group(2)
                    
                    settings.createChannel( ts, pebChnum, ch )
                    los.append('\tCH%s'%ch )
                    
                    catchWindow = False
                    for unit in re.findall( "(\d+\.?\d*)\s*(ms|ns|s)*\s*-\s*(\d+\.?\d*)\s*(ms|ns|s)*\s*([hlmx])", setup, flags=re.IGNORECASE ):
                        catchWindow = True
                        los.append("W"+'\t\t'+','.join(unit))
                        
                        t1s, u1s, t2s, u2s, lvs = unit
                        loc = toUnitTime(t1s, u1s)
                        loc2 = toUnitTime(t2s, u2s)
                        
                        settings.setWindowCompare( ts, pebChnum, ch, loc, loc2, lvs )
                    
                    if not catchWindow:
                        for unit in re.findall( "(\d+\.?\d*)\s*(ms|ns|s)*\s*([a-z])", setup, flags=re.IGNORECASE ):
                            tm, tu, act = unit
                            lowerAct = act.lower()
                            loc = toUnitTime(tm, tu)
                            
                            if   lowerAct in ChannelSetting().listDriverChars():  settings.newDriver( ts, pebChnum, ch, loc, act )
                            elif lowerAct in ChannelSetting().listCompareChars(): settings.setStrobeCompare( ts, pebChnum, ch, loc, act )
                            elif lowerAct in ChannelSetting().listIOChars():      settings.newIO( ts, pebChnum, ch, loc, act )
                            else:
                                raise Exception( 'Unknow action "%s"'%act )
                            
                            los.append('\t\t'+','.join(unit))
                except Exception as inst:
                    PrintException()
                    
                    lineno = sublineno + content.count("\n",0,mBlk.start()) + 2
                    
                    if hasattr(inst, 'message'):
                        print "Line %d: error message: %s"%(lineno, inst.message)
                    else:
                        print "Line %d:"%(lineno, "unknow error")
                    
                    print '\t"%s"'%(eline)
                    print "-"*80
                    parseFailed = True
        
    if parseFailed:
        raise Exception("Parse falied")
    
    return settings, slotTimes, slots



#--- INSTRUCTIONS ----------------------------------------------------------------
class Memory( dict ):
    def __init__( self ):
        dict.__init__( self )
        self.currslot = {}
        self.slotCurrAddress = {}
        # self.slotAndAddress = ( None, 0 )
    
    def rawWrite( self, val ):
        # if self.slotAndAddress in self: raise Exception("Slot %d address 0x%X write twice"%(self.slotAndAddress[0], self.slotAndAddress[1]))
        # if self.slotAndAddress[0] is None: raise Exception("Slot have not been set")
        # self.update({ self.slotAndAddress : val })
        # self.slotAndAddress = (self.slotAndAddress[0], self.slotAndAddress[1]+1)
        addr = self.slotCurrAddress[ self.currslot ]
        
        self.update({ ( self.currslot, addr ) : val })
        self.slotCurrAddress[ self.currslot ] = addr + 1
    
    def setSlot( self, slot ):
        if slot < 0: raise Exception("Cannot accept negtive slot %d"%slot)
            
        # self.slotAndAddress = ( slot, self.slotAndAddress[1] )
        self.currslot = slot
        if self.currslot not in self.slotCurrAddress:
            self.slotCurrAddress.update( { self.currslot : 0 } )
        
    def setAddr( self, addr ):
        if addr < 0: raise Exception("Cannot accept negtive address %d"%addr)
        # slot = self.slotAndAddress[0]
        # self.slotAndAddress = ( slot, addr )
        self.slotCurrAddress[ self.currslot ] = addr
    
    def getCurSlotAddr( self ):
        # return self.slotAndAddress
        return (self.currslot, self.slotCurrAddress[ self.currslot ])
    
    def exportEntireCsv( self ):
        csvs = []
        for slotnumAddr in sorted( self.keys() ):
            slotNum, addr = slotnumAddr
            val = self[slotnumAddr]
            # slotNum, addr, val
            csvs.append("%X,%X,%X"%( slotNum, addr, val ))
        return "\n".join(csvs)
            


class PattMem( Memory ):
    def __init__( self ):
        Memory.__init__( self )
        self.chtDefault = 7
    
    def setChtDefault( self, d ):
        if 0 <= d <= 7:
            self.chtDefault = d
        else:
            raise Exception( "Setting default index of channel failed, \"%d\" out of range"%d )
   
    def xwrite( self, dchts ):
        val = 0
        for i in xrange(64):
            val <<= 3
            val |= ( dchts[i] & 0x7 ) if i in dchts else self.chtDefault
        
        self.rawWrite( val )
    
    def write( self, dchts ):
        val = 0
        for fpga in xrange(4):
            val <<= 48
            pat = 0
            for ch in xrange(15,-1,-1):
                i = fpga*16+ch
                pat <<= 3
                pat |= ( dchts[i] & 0x7 ) if i in dchts else self.chtDefault
            val |= pat
        self.rawWrite( val )
    
    def exportEntireCsv( self ):
        # override
        csvs = []
        
        for slotnumAddr in sorted( self.keys() ):
            slotNum, addr = slotnumAddr
            val = self[slotnumAddr]
            # slotNum, addr, val
            val0 = ( val>>(48*3) ) & 0xFFFFFFFFFFFF
            val1 = ( val>>(48*2) ) & 0xFFFFFFFFFFFF
            val2 = ( val>>(48*1) ) & 0xFFFFFFFFFFFF
            val3 = ( val>>(48*0) ) & 0xFFFFFFFFFFFF
            csvs.append("%X,%X,%X,%X,%X,%X"%( slotNum, addr, val0, val1, val2, val3 ))
        return "\n".join(csvs)



class InstrMem( Memory ):
    def __init__( self ):
        Memory.__init__( self )
        
    def write( self, instr, ts, operand=None ):
        if operand is None: operand = 0
        if operand >= (1<<24) or operand < 0: raise Exception( "Operand %d out of range"%operand )
        val = ((instr&0x7)<<29) | ((ts&0x3F)<<23) | ( (operand&0x7FFFFF))
        self.rawWrite( val )


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

def parseINS( txt ):
    instrToCode = {
          'NOP' : 0
        , 'JMP' : 1
        , 'JSR' : 2
        , 'RTN' : 3
        , 'PLC' : 4
        , 'EOL' : 5
        , 'END' : 6
        , 'MCH' : 7
    # Legacy support
        , 'RPT' : 0
    }
    
    insMem = InstrMem()
    patMem = PattMem()
    dconsts = {}
    currentSlotNum = None
    instSlots = set([])
    for lineind, eline in enumerate(txt.splitlines()):
        lineno = lineind+1
        
        # Clear comments
        p = eline.find("//")
        if p != -1: eline = eline[:p]
        
        eline = eline.upper().strip()
        
        # compiler command: log (ignore)
        if re.search( "\A\s*log\s+.*", eline, flags=re.IGNORECASE ):
            continue
        
        # compiler command: slot
        mslot = re.search( "\A\s*@\s*([\t 0-9,\-]+)\s*\Z", eline, flags=re.IGNORECASE )
        if mslot:
            for slotNum in numListLine2List( mslot.group(1) ):
                if slotNum < 1: raise Exception("Slot number start from 1")
                instSlots.add( slotNum )
                continue
            
        
        # compiler command: default
        mdefaultChts = re.search("\A\s*default\s*(\S+)\s*\Z", eline, flags=re.IGNORECASE )
        if mdefaultChts:
            defChtsStr =  mdefaultChts.group(1)
            defChts = strToInt( defChtsStr )
            try:    defChts = strToInt( defChtsStr )
            except: raise Exception('Line %d: Invalid default channel index value "%s"'%( lineno, defChtsStr ))
            patMem.setChtDefault( defChts )
            continue
        
        # compiler command: constant define
        mdefine = re.search("\A\s*\[(.*)\]\s+(.*)\s*\Z", eline, flags=re.IGNORECASE  )
        if mdefine:
            valStr =  mdefine.group(1)
            nameStr = mdefine.group(2)
            
            try:    val = strToInt( valStr )
            except: raise Exception('Line %d: Invalid constant definition "%s"'%( lineno, valStr ))
            
            if nameStr in dconsts: raise Exception('Line %d: Redefine constant "%s"'%( lineno, nameStr ))
            dconsts.update( { nameStr : val })
            
            continue
        
        
        # Address
        maddr  = re.search( "\A\s*#\s*(.*)\Z", eline, flags=re.IGNORECASE  )
        if maddr:
            addrStr = maddr.group(1).strip()
            try:    addr = strToInt( addrStr )
            except: raise Exception('Line %d: Invalid address "%s"'%( lineno, addrStr ))
            
            for slot in instSlots:
                insMem.setSlot( slot )
                patMem.setSlot( slot )
                insMem.setAddr( addr )
                patMem.setAddr( addr )
            continue
        
        # General instruction format
        mbasic = re.search( "\A\s*(\w+)\s+(\w+)(.*)\Z", eline, flags=re.IGNORECASE  )
        if not mbasic: continue
        
        instrStr = mbasic.group(1)
        instr = instrToCode.get(instrStr)
        if instr is None: raise Exception('Line %d: Illegal instruction "%s"'%( lineno, instr ))
        
        tsStr = mbasic.group(2)
        try:    ts = strToInt( tsStr )
        except: raise Exception('Line %d: Invalid time slot "%s"'%( lineno, tsStr ))
        
        
        operand_chts = mbasic.group(3).strip()
        # no operand and channel indexes
        if len(operand_chts) == 0:
            for slot in instSlots:
                insMem.setSlot( slot )
                patMem.setSlot( slot )
                insMem.write( instr, ts )
                patMem.write( {} )
            continue
        
        # parse operand and channel indexes
        # NOP 0 [XXX XXX]
        
        if not operand_chts[0].isdigit():
            operandStr = None
            chtsStr    = operand_chts
        else:
            mop_chts = re.search( "\A(\S+)\s+(.*)\Z", operand_chts, flags=re.IGNORECASE  )
            if not mop_chts:
                if operand_chts[0].isdigit():
                    operandStr = operand_chts
                    chtsStr    = None
                    
                else:
                    chtsStr    = operand_chts
                    operandStr = None
                    
            else:        
                operandStr = mop_chts.group(1)
                chtsStr    = mop_chts.group(2)
        
        # Operand
        if operandStr is None:
            operand = None
        else:
            try:    operand = strToInt( operandStr )
            except: raise Exception('Line %d: Invalid operand "%s"'%( lineno, operandStr ))
        
        if len(instSlots) == 0:
            raise Exception('No slot number assigned.  Fo example, use "@1,2,3" assign slot 1, 2, 3 to be used')
        for slot in instSlots:
            insMem.setSlot( slot )
            insMem.write( instr, ts, operand )
        
        # Channel indexes
        dChIndsPerSlot = {}
        if chtsStr is not None:
            for slotNumStr, chNumStr1, chNumStr2, tscIndexStr in re.findall( "s(\d+)c(\d+)\s*-?\s*(\d*)\s*=\s*(\w+)", chtsStr, flags=re.IGNORECASE ):
            # for slotNumStr, chNumStr, tscIndexStr in re.findall( "s(\d+)c(\d+)\s*=\s*(\w+)", chtsStr, flags=re.IGNORECASE ):
                slotNum = int( slotNumStr )
                if slotNum < 1:
                    raise Exception("Slot number start from 1")
                
                chNum1 = int( chNumStr1 )
                chNum2 = int( chNumStr2 ) if len(chNumStr2) > 0 else chNum1
                
                if chNum1 < 1 or chNum2<1: raise Exception("PEB Channel number start from 1")
                
                # Make 1st channel number be the less one
                if chNum2 < chNum1:
                    tmpChNum = chNum2
                    chNum2 = chNum1
                    chNum1 = tmpChNum
                
                chInd1 = chNum1-1
                chInd2 = chNum2-1
                
                if tscIndexStr in dconsts: tscIndex =  dconsts[ tscIndexStr ]
                else: raise Exception( "Line %d: Time set selection \"%s\" not defined\n\"%s\""%( lineno, tscIndexStr, eline ) )
                
                if slotNum not in dChIndsPerSlot: dChIndsPerSlot.update( { slotNum : {} } )
                
                for chInd in xrange( chInd1, chInd2+1 ):
                    if chInd in dChIndsPerSlot[slotNum]: raise Exception( "Line %d: Time set selection \"%s\" re-defined\n\"%s\""%( lineno, chNumStr, eline ) )
                    dChIndsPerSlot[slotNum].update( { chInd : tscIndex } )
            
            if len( dChIndsPerSlot ) == 0:
                raise Exception( "Line %d: Time set selection string \"%s\" cannot be parsed\n\"%s\""%( lineno, chtsStr, eline ) )
        
        for slotNum in dChIndsPerSlot:
            patMem.setSlot(slotNum)
            patMem.write( dChIndsPerSlot[ slotNum ] )
    
    return insMem, patMem


def parseLOG( txt ):
    DEFAULT_RECOVER = 0
    DEFAULT_MODE = 0
    DEFAULT_CYCLE_COUNT = 0
    DEFAULT_VECTOR_ADDRESS = 0
    
    def mkErrMsg():
        msgs = ['Log should have 4 or less arguments: vector_address,cycle_count,mode,recover']
        msgs.append( 'vector_address: 0~0x7FFFFF' )
        msgs.append( 'cycle_count: 0~FFFFFFFF' )
        msgs.append( "mode: 'n' for normal, 'f' for fail, 'r' for repeat fold" )
        msgs.append( "recover: '1' to turn on recover mode, else '0'" )
        
        return '\n'.join(msgs) + '\n'
    
    # Scan for used slots
    logCtrlSlots = set([])
    for eline in txt.splitlines():
        eline = clearCmt( eline )
        mslot = re.search( "\A\s*@\s*([\t 0-9,\-]+)\s*\Z", eline, flags=re.IGNORECASE )
        if mslot:
            for slotNum in numListLine2List( mslot.group(1) ):
                if slotNum < 1: raise Exception("Slot number start from 1")
                logCtrlSlots.add( slotNum )
    if len( logCtrlSlots ) == 0:
        raise Exception("No slot assigned!")
    
    # Scan for log control
    for eline in txt.splitlines():
        eline = clearCmt( eline )
        m = re.search( "^\s*log\s+(.*)\s*$", eline, flags=re.IGNORECASE )
        if m:
            args = m.group(1).split(',')
            
            if len(args) > 4: raise Exception( mkErrMsg() )
            
            # Recover
            if len(args) > 3:
                recoverStr = args[3].strip()
                if recoverStr == '0':   recover = 0
                elif recoverStr == '1': recover = 1
                else:                   raise Exception( mkErrMsg() )
            else:
                recover = DEFAULT_RECOVER
            
            # Mode
            if len(args) > 2:
                modeStr = args[2].strip().lower()
                if   modeStr == 'n': mode = 0
                elif modeStr == 'f': mode = 2
                elif modeStr == 'r': mode = 3
                else:                raise Exception( mkErrMsg() )
            else:
                mode = DEFAULT_MODE
            
            # Cycle Count
            if len(args) > 1:
                cycleStr = args[1].strip()
                try:
                    cycleCount = strToInt(cycleStr)
                except:
                    raise Exception( mkErrMsg() )
                if cycleCount < 0:
                    raise Exception( mkErrMsg() )
            else:
                cycleCount = DEFAULT_CYCLE_COUNT
            
            # Vector Address
            if len(args) > 0:
                vectorStr = args[0].strip()
                try:
                    vectorAddress = strToInt(vectorStr)
                except:
                    raise Exception( mkErrMsg() )
                if vectorAddress < 0:
                    raise Exception( mkErrMsg() )
            else:
                vectorAddress = DEFAULT_VECTOR_ADDRESS
            
            break
    else: # No log setup: all use default
        vectorAddress = DEFAULT_VECTOR_ADDRESS
        cycleCount    = DEFAULT_CYCLE_COUNT
        mode          = DEFAULT_MODE
        recover       = DEFAULT_RECOVER
        
    csvLines = []
    for slotNum in sorted(logCtrlSlots):
        csvLines.append( '%X,%X,%X,%X,%X'%( slotNum, vectorAddress, cycleCount, mode, recover ) )
    return "\n".join( csvLines )


class TimeslotChannelAka(object):
    def __init__( self ):
        self.akas = {}
        self.defaultAka = {
              '0' : 0
            , '1' : 1
            , 'H' : 2
            , 'L' : 3
            , 'X' : 4
        }
    
    def addAka( self, ts, aka, ch ):
        if ts not in self.akas:
            self.akas.update( { ts : {} })
        if aka not in self.akas[ts]:
            self.akas[ts].update( { aka : ch })
    
    def setDefault( self, d ):
        self.defaultAka.clear()
        for name, ch in d.iteritems():
            self.defaultAka.update( { name : ch })
    
    def akaFindCh( self, ts, aka ):
        if ts not in self.akas or aka not in self.akas[ts]:
            if aka not in self.defaultAka:
                raise Exception('"%s" never been defined by user or by default in timeslot %d'%(aka, ts))
            else:
                return self.defaultAka[aka]
        else:
            return self.akas[ts][aka]


class InstCodeContents( object ):
    def __init__( self ):
        self.chPatPerSlot = {}
        self.instrPerSlot = {}
    
    def addInstr( self, slotNum, addr, ts, instr, operand ):
        if slotNum not in self.instrPerSlot: self.instrPerSlot.update({ slotNum : {} })
        if addr not in self.instrPerSlot[slotNum]: self.instrPerSlot[slotNum].update( { addr : {} })
        self.instrPerSlot[slotNum][addr].update( {
              'ts' : ts
            , 'instr' : instr
            , 'operand' : operand
        } )
        
    
    def addChPat( self, slotNum, addr, chNum, actChar ):
        if slotNum not in self.chPatPerSlot: self.chPatPerSlot.update({ slotNum : {} })
        if addr not in self.chPatPerSlot[slotNum]: self.chPatPerSlot[slotNum].update( { addr : {} })
        if chNum not in self.chPatPerSlot[slotNum][addr]: self.chPatPerSlot[slotNum][addr].update( { chNum : {} })
        self.chPatPerSlot[slotNum][addr].update( { chNum : actChar })
    
    def exportEntireCode( self ):
        los = []
        for slotNum in sorted( self.instrPerSlot ):
            for addr in sorted( self.instrPerSlot[slotNum] ):
                instr = self.instrPerSlot[slotNum][addr]['instr']
                ts = self.instrPerSlot[slotNum][addr]['ts']
                operand = self.instrPerSlot[slotNum][addr]['operand']
                
                chStrs = []
                for chNum, actChar in self.chPatPerSlot[slotNum][addr].iteritems():
                    chStrs.append( "s%dc%d=%s"%( slotNum, chNum, actChar ))
                
                los.append( "#%d"%addr )
                los.append( "%s %d %d %s"%(instr, ts, operand, ' '.join( chStrs )))
            los.append('')
        return "\n".join( los )
        

def parseLegacyPat( txt ):
    '''
    (LABEL:)? (INSTR (OPERAND)?)? \(PATTERN\) (TIMESLOT)
    
    '''
    columns = collections.OrderedDict()
    chAppNameMapSlotNumsChNums = {}
    labels = {}
    
    icc = InstCodeContents()
    stacks = []
    curraddr = -1
    currts = None
    slotChPool = {}
    for lineind, eline in enumerate(txt.splitlines()):
        lineno = lineind+1
        
        # Clear comments
        p = eline.find("//")
        if p != -1: eline = eline[:p]
        
        eline = eline.upper().strip()
        
        # compiler command: CLAIM
        if eline.strip() == "CLAIM": stacks.append( "CLAIM" )
        elif eline.strip() == "ENDCLAIM":
            if stacks.pop() != "CLAIM": raise Exception( "Line %d: Unmatched ENDCLAIM"%( lineno ) )
        else:
            if len(stacks) > 0 and stacks[-1] == "CLAIM":
                m = re.search( "\A\s*(\w+)\s*(\w+)\s*(\w+)\s*\Z", eline )
                slotNum, chNum, chAppName  = int(m.group( 1 )), int(m.group( 2 )), m.group( 3 ),
                
                if (slotNum, chNum) in slotChPool:
                    conflictLineno, conflictName = slotChPool[(slotNum, chNum)]
                    raise Exception( "Line %d: Slot %d channel %d cannot been claimed again as \"%s\" because it was already claimed at line %d as \"%s\""%( lineno, slotNum, chNum, chAppName, conflictLineno, conflictName ) )
                
                if chAppName not in chAppNameMapSlotNumsChNums: chAppNameMapSlotNumsChNums.update({ chAppName : {} })
                if slotNum not in chAppNameMapSlotNumsChNums[ chAppName ]:chAppNameMapSlotNumsChNums[ chAppName ].update( { slotNum : set([]) } )
                chAppNameMapSlotNumsChNums[ chAppName ][slotNum].add( chNum )
                
                slotChPool.update({(slotNum, chNum) : ( lineno, chAppName) })
                continue
        
        # compiler command: HEAD
        if eline.startswith("HEAD ") and eline.endswith(';') and ":" not in eline:
            heads = []
            for chAppName in re.findall( "(\w+)", eline )[1:]:
                if chAppName not in chAppNameMapSlotNumsChNums: raise Exception("Line %d: HEAD has undefined channel name \"%s\""%(lineno, chAppName))
                heads.append( chAppNameMapSlotNumsChNums[ chAppName ] )
            continue
        
        
        mCmd = re.search( "\A\s*((\w+)\s*:\s*)?((\w+)\s+(\w+)?)?\s*\((.*)\)\s*(TS(\d+))?\s*;", eline )
        if not mCmd:
            # Unknow line
            continue
        
        # valid command comfirmed
        curraddr += 1
        
        labelStr   = mCmd.group(2)
        instrStr   = mCmd.group(4)
        operandStr = mCmd.group(5)
        actionsStr = mCmd.group(6)
        tsStr      = mCmd.group(8)
        
        # Address or Label?
        if labelStr is not None:
            try:
                addr = strToInt(labelStr)
                curraddr = addr
            except:
                if labelStr in labels: raise Exception("Line %d: Label \"%s\" was %Xh, trying to re-define as %Xh"%( lineno, labelStr, labels[labelStr], curraddr ))
                labels.update( { labelStr : curraddr })
            
        # assign default micro-instruction if empty
        if instrStr is None:
            instrStr = 'NOP'
        
        # assign zero if no operand, or map to label's constant if it is label
        if operandStr is None:
            operand = 0
        elif operandStr in labels:
            operand = labels[operandStr]
        else:
            try:
                operand = strToInt( operandStr )
            except:
                raise Exception( "Line %d: Operand \"%s\" undefined"%( lineno, operandStr ) )
        
        
        # new timeslot?
        if tsStr is not None:
            currts = int( tsStr )
        
        if currts is None:
            raise Exception( "Line %d: The first instruction MUST assign timeslot\n\"\%s\""%( lineno, eline ) )
        
        # expand channels
        if actionsStr is None:
            raise Exception("Line %d: \"%s\" cannot parse pattern from it"%(lineno, eline))
        
        goodActionsStr = [ c for c in actionsStr if c != ' ' ]
        
        if len( goodActionsStr ) != len( heads ):
            raise Exception( "Line %d: Pattern amount %d != head amount %d\n\"%s\""%( lineno, len(goodActionsStr), len( heads ), eline ) )
        
        for act, slotsChs in zip(goodActionsStr, heads ):
            for slotNum, chNums in slotsChs.iteritems():
                icc.addInstr( slotNum, curraddr, currts, instrStr, operand )
                for chNum in chNums:
                    icc.addChPat( slotNum, curraddr, chNum, act )
        
    return icc.exportEntireCode()

#--- Channels Voltage/Current Threshold setup ------------------------------------
class SlotsChannelsThresholds( dict ):
    DONT_CHANGE_CHAR = "X"
    def __init__( self ):
        dict.__init__( self )
        self.buildTags()
    
    def buildTags( self ):
        self.tagsInfos = collections.OrderedDict()
        
        self.tagsInfos.update( { 'vih'     : "DUT's VIH (V)" } )
        self.tagsInfos.update( { 'vil'     : "DUT's VIL (V)" } )
        self.tagsInfos.update( { 'voh'     : "DUT's VOL (V)" } )
        self.tagsInfos.update( { 'vol'     : "DUT's VOL (V)" } )
        self.tagsInfos.update( { 'ioh'     : "Active load current (A), -0.006 ~ +0.012" } )
        self.tagsInfos.update( { 'iol'     : "Active load current (A), -0.006 ~ +0.012" } )
        self.tagsInfos.update( { 'load_en' : "1/0 to switch active load on/off" } )
        self.tagsInfos.update( { 'vch'     : "Clamp voltage to not exceed this level (V)" } )
        self.tagsInfos.update( { 'vcl'     : "Clamp voltage to not underneath this level (V)" } )
        self.tagsInfos.update( { 'vt'      : "VCOM, for active load (V)" } )
        self.tagsInfos.update( { 'ovdh'    : "Over voltage detect level (V)" } )
        self.tagsInfos.update( { 'ovdl'    : "Under voltage detect level (V)" } )
        
        self.tags = tuple( self.tagsInfos.keys() )
    
    def add( self, slot, ch, tag, val ):
        k = (slot, ch) 
        if k not in self:
            self.update( { k : {} } )
        self[k].update({ tag : val })
    
    def getTagsInfos( self ):
        return self.tagsInfos
    
    def getTags( self ):
        return self.tags
    
    # def xgetTags( self ):
        # return ( 'vih', 'vil', 'voh', 'vol', 'ioh', 'iol', 'load_en', 'vch', 'vcl', 'vt', 'ovdh', 'ovdl' )
    
    def getCsvColumns( self ):
        columns = ['slot', 'ch'] + list(self.getTags())
        return tuple(columns)
        
    def exportEntireCsv( self ):
        csvs = []
        for slotCh in sorted(self):
            slot, ch = slotCh
            items = [str(slot),str(ch)]
            for tag in self.getTags():
                val = self[slotCh].get(tag)
                items.append( str(val) if val is not None else SlotsChannelsThresholds.DONT_CHANGE_CHAR )   # DONT_CHANGE_CHAR is employed instead of leaving an empty field because cpp scanf doesn't handle empty sub-scanset
            csvs.append( ",".join( items ))
        return "\n".join(csvs)


class ErrorLogs( object ):
    def __init__( self ):
        self.logs = collections.OrderedDict()
    
    def add( self, pos, msg ):
        if pos not in self.logs:
            self.logs.update( { pos : [] })
        self.logs[pos].append( msg )
    
    def export( self, txt ):
        los = []
        llines = [ eline for eline in txt.splitlines() ]
        
        for pos, msgs in self.logs.iteritems():
            lineno = txt[:pos].count('\n') + 1
            los.append( 'Line %d: "%s"'%(lineno, llines[lineno-1]) )
            for msg in msgs:
                los.append( "    %s"%msg )
        return '\n'.join(los)



def helpTH():
    los = []
    
    los.append( "// Use this script to configure every channels' levels of drivers, loads and comparators " )
    los.append( "" )
    los.append( "Template:" )
    los.append( "" )
    los.append( "slot:<slot_number> ch: <channels>" )
    los.append( "{" )
    for tag, info in SlotsChannelsThresholds().getTagsInfos().iteritems():
        los.append( "    %12s = <%s>"%( tag, info ))
    los.append( "}" )
    los.append( "Example: Configure channel 1,2,3 on slot 4" )
    los.append( "slot:4 ch: 1,2,3" )
    los.append( "{" )
    los.append( "    // Setup dut's VIH, VIL" )
    los.append( "    vih = 4.2" )
    los.append( "    vil = 0.8" )
    los.append( "    // Other levels will not change" )
    los.append( "}" )
    los.append( "" )
    
    return "\n\n" + "\n// ".join( los )
    
    
def parseTH( txt ):
    '''
    ch:1,6,7,8,10
    {
        vih = 5
        vil = 0.5
        voh = 4
        vol = 1
    }
    '''
    clearTxt = clearCmt(txt)
    
    blkParser = re.compile( "slot\s*:\s*([\t 0-9,\-]+)\s+ch\s*:([\t 0-9,\-]+)\s*\{([^\}]*)\}", flags=re.IGNORECASE|re.MULTILINE|re.DOTALL )
    
    chth = SlotsChannelsThresholds()
    errl = ErrorLogs()
    errorCnt = 0
    validBlkCnt = 0
    validThCnt = 0
    for mBlk in blkParser.finditer(clearTxt):
        validBlkCnt += 1
        slotsStr, chsStr, thsStr = mBlk.groups()
        posbase = mBlk.start(3)
        
        slots = numListLine2List( slotsStr )
        chs   = numListLine2List( chsStr )
        
        thParser = re.compile( "\s*(\w+)\s*=\s*(-?\s*[0-9]\.?[0-9]*)\s", flags=re.IGNORECASE )
        for mTh in thParser.finditer( thsStr ):
            tagStr, thValStr = mTh.groups()
            pos = mTh.start(2) + posbase
            
            if tagStr not in chth.getTags():
                errl.add( pos, 'Illegal tag "%s"'%tagStr )
                errorCnt += 1
                continue
            
            try:
                thVal = float(thValStr)
            except:
                errl.add( pos, 'Illegal value "%s" of tag "%s"'%( thValStr, tagStr ))
                errorCnt += 1
                continue
            
            validThCnt += 1
            for slot in slots:
                if slot < 1: raise Exception("PEB slot must start from 1")
                for ch in chs:
                    if ch < 1: raise Exception("PEB channel must start from 1")
                    chth.add( slot, ch, tagStr, thVal )
    # print errorCnt
    # print validThCnt, validBlkCnt
    if errorCnt != 0:
        print "Total errors: %d"%errorCnt
        print errl.export(clearTxt)
        raise Exception( "Invalid contents" )
    
    return chth
            
        
        


#-----SITE ASSIGN--------------------------------------------------------------

class Sites( object ):
    def __init__( self ):
        '''
        sitesEnables: {
              ( slot_number, site_number ) : site_link_bits
            , ...
        }
        '''
        self.sitesEnables = {}
    
    def add( self, slotNum, siteNum, chnFpgaNum, linkNum ):
        key = ( slotNum, siteNum )
        if key not in self.sitesEnables: self.sitesEnables.update( { key : 0 })
        self.sitesEnables[ key ] |= (1<<( (chnFpgaNum-1)*2 + linkNum-1 ))
    
    def exportEntireCsv( self ):
        '''
        ----------------------------------------------
        Main FPGA Registers
        ----------------------------------------------
        0x10:   Site-1 Enable Bits
        __________________________________________________________________________________
        | Bit    | 7      | 6      | 5      | 4      | 3      | 2      | 1      | 0      |
        |________|________|________|________|________|________|________|________|________|
        | Desc   | Site-1 | Site-1 | Site-1 | Site-1 | Site-1 | Site-1 | Site-1 | Site-1 |
        |        | Enable | Enable | Enable | Enable | Enable | Enable | Enable | Enable |
        |        | FPGA 4 | FPGA 4 | FPGA 3 | FPGA 3 | FPGA 2 | FPGA 2 | FPGA 1 | FPGA 1 |
        |        | Link 2 | Link 1 | Link 2 | Link 1 | Link 2 | Link 1 | Link 2 | Link 1 |
        |________|________|________|________|________|________|________|________|________|
        
        0x11:   Site-2 Enable Bits
        0x12:   Site-3 Enable Bits
        ...
        0x1F:   Site-16 Enable Bits
        
        
        ----------------------------------------------
        Main FPGA CSV Fields
        ----------------------------------------------
        ___________________________________________________
        | Index  |          0 |          1 |            2 |
        |________|____________|____________|______________|
        | Desc   | SlotNumber | SiteNumber | SiteLinkBits |
        |________|____________|____________|______________|
        
        '''
        csvlines = []
        for slotNum, siteNum in sorted( self.sitesEnables.keys() ):
            linkbits = self.sitesEnables[ ( slotNum, siteNum )]
            csvlines.append( "%X,%X,%X"%( slotNum, siteNum, linkbits ))
        return "\n".join( csvlines )


class Links( object ):
    LINK_AMOUNT = 2
    def __init__( self ):
        self.linkbits = {}
    
    def add( self, slotNum, chnFpgaNum, fpgaChnNum, siteName ):
        key = ( slotNum, chnFpgaNum )
        if key not in self.linkbits: self.linkbits.update( { key : collections.OrderedDict() })
        if siteName not in self.linkbits[ key ]:
            if len( self.linkbits[ key ] ) >= Links.LINK_AMOUNT:
                raise Exception( "Link fulled" )
            self.linkbits[ key ].update( { siteName : 0 } )
        
        self.linkbits[ key ][ siteName ] |= (1<<(fpgaChnNum-1))
        
        linkNum = self.linkbits[ key ].keys().index( siteName ) + 1
        
        return linkNum
    
    def exportEntireCsv( self ):
        '''
        ----------------------------------------------
        Channel FPGA
        ----------------------------------------------
        0x12:   Fail Link-1 Enable Bits
        ______________________________________________
        |Bit  |15     |14     |...   |1      |0      |
        |_____|_______|_______|______|_______|_______|
        |Desc |Ch16   |Ch15   |...   |Ch2    |Ch1    |
        |     |Link   |Link   |      |Link   |Link   |
        |     |Site-1 |Site-1 |      |Site-1 |Site-1 |
        |_____|_______|_______|______|_______|_______|
        
        0x13:   Fail Link-2 Enable Bits
        _______________________________________________________
        | Bit    | 15     | 14     | ...    | 1      | 0      |
        |________|________|________|________|________|________|
        | Desc   | Ch16   | Ch15   | ...    | Ch2    | Ch1    |
        |        | Link   | Link   |        | Link   | Link   |
        |        | Site-2 | Site-2 |        | Site-2 | Site-2 |
        |________|________|________|________|________|________|
        
        
        ----------------------------------------------
        Channel FPGA CSV Fields
        ----------------------------------------------
        ____________________________________________________________
        | Index  |          0 |          1 |         2 |         3 |
        |________|____________|____________|___________|___________|
        | Desc   | SlotNumber | ChnFpgaNum | Link1Bits | Link2Bits |
        |________|____________|____________|___________|___________|
        
        '''
        csvlines = []
        for slotNum, chnFpgaNum in sorted( self.linkbits.keys()):
            items = [ "%X"%slotNum, "%X"%chnFpgaNum ]
            linkCount = 0
            for linkbits in self.linkbits[( slotNum, chnFpgaNum )].values():
                items.append( "%X"%linkbits )
                linkCount += 1
            # pad unused link
            for _ in xrange( Links.LINK_AMOUNT - linkCount ): items.append( '0' )
            csvlines.append( ",".join( items ))
        return "\n".join( csvlines )


class MatchSites( object ):
    def __init__( self ):
        self.failSites = []
        self.slotMatchSites = {}
    
    def add( self, slotNum, failSiteNum, chnFpgaNum ):
        if failSiteNum not in self.failSites: self.failSites.append( failSiteNum )
        
        matchSiteNum = self.failSites.index( failSiteNum ) + 1
        key = ( slotNum, matchSiteNum )
        chnFpgaNum
        if key not in self.slotMatchSites: self.slotMatchSites.update( { key : set() } )
        self.slotMatchSites[ key ].add( chnFpgaNum )
    
    def exportEntireCsv( self ):
        csvlines = []
        # for ( slotNum, matchSiteNum ), chnFpgaNums in self.slotMatchSites.iteritems():
        for key in sorted( self.slotMatchSites ):
            ( slotNum, matchSiteNum ) = key
            chnFpgaNums = self.slotMatchSites[ key ]
            
            chnFpgaBits = 0
            for num in chnFpgaNums:
                chnFpgaBits |= 1<<(num-1)
            
            # SlotNum, MatchSiteNum, ChnFPGAEnableBits
            csvlines.append( "%X,%X,%X"%( slotNum, matchSiteNum, chnFpgaBits ))
        return '\n'.join( csvlines )


class MatchChannels( object ):
    def __init__( self ):
        self.slotChnFpgaChns = {}
    
    def add( self, slotNum, chnFpgaNum, fpgaChnNum ):
        key = ( slotNum, chnFpgaNum )
        
        if key not in self.slotChnFpgaChns: self.slotChnFpgaChns.update( { key : set() } )
        self.slotChnFpgaChns[ key ].add( fpgaChnNum )
    
    def exportEntireCsv( self ):
        csvlines = []
        
        for key in sorted( self.slotChnFpgaChns ):
            fpgaChnNums = self.slotChnFpgaChns[ key ]
            ( slotNum, chnFpgaNum ) = key
            
            
            fpgaChnBits = 0
            for fpgaChnNum in fpgaChnNums:
                fpgaChnBits |= 1<<(fpgaChnNum-1)
            
            csvlines.append( "%X,%X,%X"%( slotNum, chnFpgaNum, fpgaChnBits ))
        return '\n'.join( csvlines )


class FailBus( object ):
    def __init__( self ):
        self.sites = Sites()
        self.links = Links()
        self.matchsites = MatchSites()
        self.matchch = MatchChannels()
    
    def exportLinksCsv( self ):
        return self.links.exportEntireCsv()
    
    def exportSitesCsv( self ):
        return self.sites.exportEntireCsv()
    
    def exportMatchSiteCsv( self ):
        return self.matchsites.exportEntireCsv()
    
    def exportMatchChannelCsv( self ):
        return self.matchch.exportEntireCsv()
    
    def feed( self, line ):
        line = clearCmt( line )
        m = re.search( "\A\s*(\d+)\s*=\s*(.*)\s*\Z", line )
        if m:
            siteNum = int( m.group(1) )
            linkChsStr = m.group(2)
            for slotNumsStr, pebChnNumsStr in re.findall( "s([0-9\-]+)\s*c([0-9\-]+)", linkChsStr, flags=re.IGNORECASE ):
                slotNums   = numListLine2List( slotNumsStr )
                pebChnNums = numListLine2List( pebChnNumsStr )
                
                for slotNum in slotNums:
                    for pebChnNum in pebChnNums:
                        chnFpgaNum = (pebChnNum-1)/16 + 1
                        fpgaChnNum = ((pebChnNum-1)&0xF) + 1
                        
                        linkNum = self.links.add( slotNum, chnFpgaNum, fpgaChnNum, siteNum )
                        self.sites.add( slotNum, siteNum, chnFpgaNum, linkNum )
                        self.matchsites.add( slotNum, siteNum, chnFpgaNum )
                        self.matchch.add( slotNum, chnFpgaNum, fpgaChnNum )
            return True
        return False


def parseSITE( txt ):
    failBus = FailBus()
    for lineind, eline in enumerate(txt.splitlines()):
        lineno = lineind+1
        try:
            failBus.feed( eline )
        except Exception as inst:
            PrintException()
            if hasattr(inst, 'message'):
                print ( "Line %d: error message: %s"%(lineno, inst.message) )
            else:
                print ( "Line %d:"%(lineno, "unknow error"))
            
            print ( '\t"%s"'%(eline))
            print ( "-"*80 )
            raise
    return failBus



#--- EASY SCRIPT -----------------------------------------------------------------
def parseES( txt ):
    '''Aaron's script parser that generate TS, INST and PATT altogether by using
    a specified language
    
    #define    
    @2
    
    [ ch1, ch3, ch64 ] = i[0:2]
    
    
    
    '''
    pass


#--------------------------------------------------------------------------------

def getLegacyInsTxt():
    los = []
    los.append( "default 0" )
    los.append( "[0] 0" )
    los.append( "[1] 1" )
    los.append( "[2] Z" )
    los.append( "[3] H" )
    los.append( "[4] L" )
    los.append( "[5] X" )
    return '\n'.join(los) + '\n'

if __name__ == "__main__":    
    success = False
    sys.stdout = logout
    try:
        fpath = "?"
        if len( sys.argv ) < 3:
            raise Exception("Argument should contain 2 arguments, the first is file path and the second is file tpye TS/TH/INS")
        
        fpath = sys.argv[1]
        
        with open( fpath, 'r' ) as rstream:
            txt = rstream.read()
        
        ftype = sys.argv[2]
        
        # app layer
        if ftype == 'LEGACY':
            txt = parseLegacyPat( txt )
            txt = getLegacyInsTxt() + txt
            with open("legacy_parse.txt", 'w') as wstream:
                wstream.write(txt)
            ftype = 'INS'
        
        
        if ftype == 'TS':
            settings, slotTimes, slots = parseTS( txt )
            
            csvTxt = settings.exportEntireCsv()
            print "+Time slot period settings"
            print "SET,PERIOD"
            for ts, tm in slotTimes.iteritems():
                for slot in slots:
                    print "%X,%X,%X"%(slot,ts,tm)
            print "-"
            print "+Time slot channel settings"
            print csvTxt
            print "-"
            success = True
        elif ftype == 'INS':
            insMem, patMem = parseINS( txt )
            logSettingCsv = parseLOG( txt )
            
            print "+Instruction"
            print insMem.exportEntireCsv()
            print "-"
            print "+Pattern"
            print patMem.exportEntireCsv()
            print "-"
            print "+Log settings"
            print logSettingCsv
            print "-"
            
            success = True
        elif ftype == 'TH':
            chth = parseTH(txt)
            print "+PEB Channel level settings"
            print ",".join(chth.getCsvColumns())
            print chth.exportEntireCsv()
            print "-"
            success = True
        elif ftype == 'FB':
            failBus = parseSITE(txt)
            print "+Sites"
            print failBus.exportSitesCsv()
            print "-"
            print "+Links"
            print failBus.exportLinksCsv()
            print "-"
            print "+MatchSites"
            print failBus.exportMatchSiteCsv()
            print "-"
            print "+MatchCh"
            print failBus.exportMatchChannelCsv()
            print "-"
            success = True
            
        elif ftype == 'APPEND_TEMPLATE_TS':
            with open( fpath, 'a' ) as astream:
                astream.write( helpTS() )
            success = 1
        elif ftype == 'APPEND_TEMPLATE_TH':
            with open( fpath, 'a' ) as astream:
                astream.write( helpTH() )
            success = 1
        else:
            raise Exception("Invalid file tpye \"%s\""%(ftype))
    except:
        if _DEBUG:
            print ""
            print ""
            print "-"*80
            print traceback.format_exc()
        
        import os
        try:
            path = os.path.abspath( fpath )
        except:
            pass
        print 'Faild to parse pattern "%s"'%( path )
    
    sys.stdout = old_stdout
    if success:
        print "P"   # for Pass
    else:
        print "E"   # for Error
    print logout.getvalue()
    if not success:
        with open("parser_error_message.txt", 'w') as wstream:
            wstream.write( logout.getvalue() )
        import os
        os.system( "parser_error_message.txt" )



