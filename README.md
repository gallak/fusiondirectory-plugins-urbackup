# fusiondirectory-plugins-urbackup

This a plugin wich could connect to the excellent urbackup system  https://urbackup.org ( thanks Martin ! you saved some scientist life !!)
This plugin make the links between FusionDirectory ( https://www.fusiondirectory.org ) and urbackup.
it isn't an alternative to the main web interface of urbackup, just a technical view for IT services

## Reporting

Its allows to follow backup per system / see old backup / current backup

## Actions

In the future, it will allow to lauch a full / incremental  file/image backup

## licence
GPL V3 licence, as usual No warranty

## in the future

We need to rewrite the php library for connecting to urbackup server
We need to implement a more user friendly interface

## Installation

trough fusiondirectory plugin manager

### files

`root@fd-14-dev:/usr/local/src/gallak# ./pluginsmanager/fusiondirectory-plugin-manager --set-fd_home=/usr/local/share/fusiondirectory  --plugin-name=urbackup --plugins-archive=/usr/local/src/gallak/ --install-plugin
Setting fd_home to /usr/local/share/fusiondirectory
plugin urbackup will be used
Plugins folder /usr/local/src/gallak/ will be used
Installing FusionDirectory's plugins
Installing plugin urbackup
Create plugin record
Scanning and update Class.cache and translations
root@fd-14-dev:/usr/local/src/gallak#
`

### schema

`
root@fd-14-dev:/usr/local/src/gallak# fusiondirectory-insert-schema -m /usr/local/share/fusiondirectory/contrib/openldap/urbackup-fd.schema

SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
executing 'ldapmodify -Y EXTERNAL -H ldapi:/// -f /usr/local/share/fusiondirectory/contrib/openldap/urbackup-fd_update.ldif'
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
modifying entry "cn={34}urbackup-fd,cn=schema,cn=config"

root@fd-14-dev:/usr/local/src/gallak#
`
