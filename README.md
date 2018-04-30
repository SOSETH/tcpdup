# TCPDup
...a duplicator for TCP streams.

## Why would you need this?
In a clustered Icinga2 setup, when exporting metrics using the Graphite module,
each performance metric is only written by a single master. If you now want to
duplicate the data to all masters, you'll need to duplicate the stream, which
is why we have TCPDup.
TCPDup simply listens on a socket, accepts connections and will send all data
to all backends, responses from a backend are silently ignored.

## Usage
```
You need to specify at least one backend!
Options:
  --help                  print this message
  --backend arg           backend to use ("x.x.x.x:port")
  --buf arg (=8192)       Number of measurements to buffer in case of
                          connectivity issues
  --port arg (=12345)     port to bind to
  --bind arg (=127.0.0.1) IP to bind to
```
Backend should probably be specified multiple times, the rest is pretty
self-explanatory

## Building
Assuming that you don't want to copy the binary around, you can build a debian
package using
```
dpkg-buildpackage -us -uc
```
The command-line options are then set in `/etc/default/tcpdup` and a systemd
unit is included.
