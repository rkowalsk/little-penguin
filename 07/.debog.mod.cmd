savedcmd_/root/little-penguin/07/debog.mod := printf '%s\n'   debog.o | awk '!x[$$0]++ { print("/root/little-penguin/07/"$$0) }' > /root/little-penguin/07/debog.mod
