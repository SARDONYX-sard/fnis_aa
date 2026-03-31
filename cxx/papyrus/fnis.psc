Scriptname FNIS Hidden
int Function set_AACondition(actor ac, string AAtype, string mod, int AAcond, int AAdebug = 1) global native
Function AAReport(string longReport, string shortReport, int AAdebug = 0, bool isError = true) global native
bool function IsGenerated() global native

string function VersionToString(bool abCreature = false) global native
int function VersionCompare(int iCompMajor, int iCompMinor1, int iCompMinor2, bool abCreature = false) global native
int function GetMajor(bool abCreature = false) global native
int function GetMinor1(bool abCreature = false) global native
int function GetMinor2(bool abCreature = false) global native
Bool function IsRelease(bool abCreature = false) global native
