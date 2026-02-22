using AdsWrapper;

var localIp = "192.168.1.43";

var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
ushort remotePort = 851;

var ads = new AdsDeviceWrapper(localIp, remoteIp, remotePort, remoteName, remoteUser, remotePassword);

var state = ads.GetState();