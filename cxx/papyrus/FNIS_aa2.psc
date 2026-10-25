ScriptName FNIS_aa2

; Native stub — implementation provided by fnis_aa.dll via SKSE.
; This file overwrites the FNIS generator output via MO2 load order.

Int Function GetAAnumber(Int listType) Global Native

String[] Function GetAAprefixList(Int nMods, String mod, Bool debugOutput = False) Global Native

String[] Function GetAAsetList(Int nSets, String mod, Bool debugOutput = False) Global Native
