# Interactive shell for endpoint 
The Endpoint interactive shell is a diagnostic tool capable of executing specific commands to the device. The interactive shell had integrated some useful commands such as the test tool of radio connection,  diagnostic commands of internet connection, AT command adapter and the test tool of TA transaction.

# How to build and install endpoint shell 
First you need to install the leaf tool. See `docs/endpoint.md`
Then build the shell app.
```
leaf shell
mkapp -t wp77xx endpoint/shell.adef/
```
Change the `wp77xx` for your specific target.
Install shell app
```
update shell.wp77xx.update
```
Connect shell
```
sudo screen /dev/ttyUSB3 9600
```
You should check your specific ttyUSB interface.

Use `help` to list all support commands.
```
> help
```
