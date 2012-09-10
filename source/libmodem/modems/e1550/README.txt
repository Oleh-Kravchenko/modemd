AT^SYSCFG=$mode,$acqOrder,$band,$roam,$srvDomain

$mode
2=Auto-Select
13=GSM only
14=WCDMA only
16=no Change

$acqOrder
0=Automatic
1=GSM prefered
2=WCDMA prefered
3=no Change

$band
3fffffff = All
other (query list with "AT^SYSCFG=?")

$roam
0=Not Supported
1=Supported
2=no Change

$srvDomain
0=Circuit-Switched only
1=Packet-Switched only
2=Circuit- & Packet-Switched
3=Any
4=no Change