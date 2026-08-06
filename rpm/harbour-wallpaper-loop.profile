# harbour-wallpaper-loop application profile

include /etc/sailjail/permissions/Base.permission
include /etc/sailjail/permissions/UserDirs.permission
include /etc/sailjail/permissions/Downloads.permission
include /etc/sailjail/permissions/Pictures.permission
include /etc/sailjail/permissions/Ambience.permission
include /etc/sailjail/permissions/Compatibility.permission

# Staged scaled wallpaper written before setAmbience.
noblacklist ${HOME}/.cache/harbour-wallpaper-loop
