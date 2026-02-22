using AdsWrapper;

var localIp = "192.168.1.43";

var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
ushort remotePort = 851;

var ads = new AdsHandler(localIp);

ads.AddRemoteRoute(remoteIp, remoteName, remoteUser, remotePassword);

ads.SetupRoute(remotePort);

ads.PrintState();