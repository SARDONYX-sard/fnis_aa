ScriptName FNIS_aa2

; Native stub — implementation provided by fnis_aa.dll via SKSE.
; This file overwrites the FNIS generator output via MO2 load order.

int Function GetAAnumber(int listType) global native

string[] Function GetAAprefixList(int nMods, string mod, bool debugOutput = false) global native

string[] Function GetAAsetList(int nSets, string mod, bool debugOutput = false) global native
