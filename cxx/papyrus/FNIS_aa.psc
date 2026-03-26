ScriptName FNIS_aa

; --- Native functions (C++ / SKSE Plugin) ---
bool Function SetAnimGroup(actor ac, string animGroup, int base, int number, string mod, bool debugOutput = false) global native
bool Function SetAnimGroupEX(actor ac, string animGroup, int base, int number, string mod, bool debugOutput = false, bool skipForce3D = false) global native
int Function GetAAmodID(string myAAprefix, string mod, bool debugOutput = false) global native
int Function GetGroupBaseValue(int AAmodID, int AAgroupID, string mod, bool debugOutput = false) global native
int[] Function GetAllGroupBaseValues(int AAmodID, string mod, bool debugOutput = false) global native
int Function GetInstallationCRC() global native
Function GetAAsets(int nSets, int[] GroupId, int[] ModId, int[] Base, int[] Index, string mod, bool debugOutput = false) global native

; --- Group ID Constants ---

; Return 0
int Function _mtidle() global
	return 0
endFunction

; Return 1
int Function _1hmidle() global
	return 1
endFunction

; Return 2
int Function _2hmidle() global
	return 2
endFunction

; Return 3
int Function _2hwidle() global
	return 3
endFunction

; Return 4
int Function _bowidle() global
	return 4
endFunction

; Return 5
int Function _cbowidle() global
	return 5
endFunction

; Return 6
int Function _h2hidle() global
	return 6
endFunction

; Return 7
int Function _magidle() global
	return 7
endFunction

; Return 8
int Function _sneakidle() global
	return 8
endFunction

; Return 9
int Function _staffidle() global
	return 9
endFunction

; Return 10
int Function _mt() global
	return 10
endFunction

; Return 11
int Function _mtx() global
	return 11
endFunction

; Return 12
int Function _mtturn() global
	return 12
endFunction

; Return 13
int Function _1hmmt() global
	return 13
endFunction

; Return 14
int Function _2hmmt() global
	return 14
endFunction

; Return 15
int Function _bowmt() global
	return 15
endFunction

; Return 16
int Function _magmt() global
	return 16
endFunction

; Return 17
int Function _magcastmt() global
	return 17
endFunction

; Return 18
int Function _sneakmt() global
	return 18
endFunction

; Return 19
int Function _1hmatk() global
	return 19
endFunction

; Return 20
int Function _1hmatkpow() global
	return 20
endFunction

; Return 21
int Function _1hmblock() global
	return 21
endFunction

; Return 22
int Function _1hmstag() global
	return 22
endFunction

; Return 23
int Function _2hmatk() global
	return 23
endFunction

; Return 24
int Function _2hmatkpow() global
	return 24
endFunction

; Return 25
int Function _2hmblock() global
	return 25
endFunction

; Return 26
int Function _2hmstag() global
	return 26
endFunction

; Return 27
int Function _2hwatk() global
	return 27
endFunction

; Return 28
int Function _2hwatkpow() global
	return 28
endFunction

; Return 29
int Function _2hwblock() global
	return 29
endFunction

; Return 30
int Function _2hwstag() global
	return 30
endFunction

; Return 31
int Function _bowatk() global
	return 31
endFunction

; Return 32
int Function _bowblock() global
	return 32
endFunction

; Return 33
int Function _h2hatk() global
	return 33
endFunction

; Return 34
int Function _h2hatkpow() global
	return 34
endFunction

; Return 35
int Function _h2hstag() global
	return 35
endFunction

; Return 36
int Function _magatk() global
	return 36
endFunction

; Return 37
int Function _1hmeqp() global
	return 37
endFunction

; Return 38
int Function _2hweqp() global
	return 38
endFunction

; Return 39
int Function _2hmeqp() global
	return 39
endFunction

; Return 40
int Function _axeeqp() global
	return 40
endFunction

; Return 41
int Function _boweqp() global
	return 41
endFunction

; Return 42
int Function _cboweqp() global
	return 42
endFunction

; Return 43
int Function _dageqp() global
	return 43
endFunction

; Return 44
int Function _h2heqp() global
	return 44
endFunction

; Return 45
int Function _maceqp() global
	return 45
endFunction

; Return 46
int Function _mageqp() global
	return 46
endFunction

; Return 47
int Function _stfeqp() global
	return 47
endFunction

; Return 48
int Function _shout() global
	return 48
endFunction

; Return 49
int Function _magcon() global
	return 49
endFunction

; Return 50
int Function _dw() global
	return 50
endFunction

; Return 51
int Function _jump() global
	return 51
endFunction

; Return 52
int Function _sprint() global
	return 52
endFunction

; Return 53
int Function _shield() global
	return 53
endFunction
