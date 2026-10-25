ScriptName FNIS_aa

; --- Native functions (C++ / SKSE Plugin) ---
Bool Function SetAnimGroup(actor ac, String animGroup, Int base, Int number, String mod, Bool debugOutput = False) Global Native
Bool Function SetAnimGroupEX(actor ac, String animGroup, Int base, Int number, String mod, Bool debugOutput = False, Bool skipForce3D = False) Global Native
Int Function GetAAmodID(String myAAprefix, String mod, Bool debugOutput = False) Global Native
Int Function GetGroupBaseValue(Int AAmodID, Int AAgroupID, String mod, Bool debugOutput = False) Global Native
Int[] Function GetAllGroupBaseValues(Int AAmodID, String mod, Bool debugOutput = False) Global Native
Int Function GetInstallationCRC() Global Native
Function GetAAsets(Int nSets, Int[] GroupId, Int[] ModId, Int[] Base, Int[] Index, String mod, Bool debugOutput = False) Global Native

; --- Group ID Constants ---

; Return 0
Int Function _mtidle() Global
    Return 0
EndFunction

; Return 1
Int Function _1hmidle() Global
    Return 1
EndFunction

; Return 2
Int Function _2hmidle() Global
    Return 2
EndFunction

; Return 3
Int Function _2hwidle() Global
    Return 3
EndFunction

; Return 4
Int Function _bowidle() Global
    Return 4
EndFunction

; Return 5
Int Function _cbowidle() Global
    Return 5
EndFunction

; Return 6
Int Function _h2hidle() Global
    Return 6
EndFunction

; Return 7
Int Function _magidle() Global
    Return 7
EndFunction

; Return 8
Int Function _sneakidle() Global
    Return 8
EndFunction

; Return 9
Int Function _staffidle() Global
    Return 9
EndFunction

; Return 10
Int Function _mt() Global
    Return 10
EndFunction

; Return 11
Int Function _mtx() Global
    Return 11
EndFunction

; Return 12
Int Function _mtturn() Global
    Return 12
EndFunction

; Return 13
Int Function _1hmmt() Global
    Return 13
EndFunction

; Return 14
Int Function _2hmmt() Global
    Return 14
EndFunction

; Return 15
Int Function _bowmt() Global
    Return 15
EndFunction

; Return 16
Int Function _magmt() Global
    Return 16
EndFunction

; Return 17
Int Function _magcastmt() Global
    Return 17
EndFunction

; Return 18
Int Function _sneakmt() Global
    Return 18
EndFunction

; Return 19
Int Function _1hmatk() Global
    Return 19
EndFunction

; Return 20
Int Function _1hmatkpow() Global
    Return 20
EndFunction

; Return 21
Int Function _1hmblock() Global
    Return 21
EndFunction

; Return 22
Int Function _1hmstag() Global
    Return 22
EndFunction

; Return 23
Int Function _2hmatk() Global
    Return 23
EndFunction

; Return 24
Int Function _2hmatkpow() Global
    Return 24
EndFunction

; Return 25
Int Function _2hmblock() Global
    Return 25
EndFunction

; Return 26
Int Function _2hmstag() Global
    Return 26
EndFunction

; Return 27
Int Function _2hwatk() Global
    Return 27
EndFunction

; Return 28
Int Function _2hwatkpow() Global
    Return 28
EndFunction

; Return 29
Int Function _2hwblock() Global
    Return 29
EndFunction

; Return 30
Int Function _2hwstag() Global
    Return 30
EndFunction

; Return 31
Int Function _bowatk() Global
    Return 31
EndFunction

; Return 32
Int Function _bowblock() Global
    Return 32
EndFunction

; Return 33
Int Function _h2hatk() Global
    Return 33
EndFunction

; Return 34
Int Function _h2hatkpow() Global
    Return 34
EndFunction

; Return 35
Int Function _h2hstag() Global
    Return 35
EndFunction

; Return 36
Int Function _magatk() Global
    Return 36
EndFunction

; Return 37
Int Function _1hmeqp() Global
    Return 37
EndFunction

; Return 38
Int Function _2hweqp() Global
    Return 38
EndFunction

; Return 39
Int Function _2hmeqp() Global
    Return 39
EndFunction

; Return 40
Int Function _axeeqp() Global
    Return 40
EndFunction

; Return 41
Int Function _boweqp() Global
    Return 41
EndFunction

; Return 42
Int Function _cboweqp() Global
    Return 42
EndFunction

; Return 43
Int Function _dageqp() Global
    Return 43
EndFunction

; Return 44
Int Function _h2heqp() Global
    Return 44
EndFunction

; Return 45
Int Function _maceqp() Global
    Return 45
EndFunction

; Return 46
Int Function _mageqp() Global
    Return 46
EndFunction

; Return 47
Int Function _stfeqp() Global
    Return 47
EndFunction

; Return 48
Int Function _shout() Global
    Return 48
EndFunction

; Return 49
Int Function _magcon() Global
    Return 49
EndFunction

; Return 50
Int Function _dw() Global
    Return 50
EndFunction

; Return 51
Int Function _jump() Global
    Return 51
EndFunction

; Return 52
Int Function _sprint() Global
    Return 52
EndFunction

; Return 53
Int Function _shield() Global
    Return 53
EndFunction
