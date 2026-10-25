ScriptName FNIS Hidden
Int Function set_AACondition(actor ac, String AAtype, String mod, Int AAcond, Int AAdebug = 1) Global Native
Function AAReport(String longReport, String shortReport, Int AAdebug = 0, Bool isError = True) Global Native
Bool Function IsGenerated() Global Native

String Function VersionToString(Bool abCreature = False) Global Native
Int Function VersionCompare(Int iCompMajor, Int iCompMinor1, Int iCompMinor2, Bool abCreature = False) Global Native
Int Function GetMajor(Bool abCreature = False) Global Native
Int Function GetMinor1(Bool abCreature = False) Global Native
Int Function GetMinor2(Bool abCreature = False) Global Native
Int Function GetFlags(Bool abCreature = False) Global Native
Bool Function IsRelease(Bool abCreature = False) Global Native
