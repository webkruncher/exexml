

[ -f testunit ] && rm testunit
make
if [ -f testunit ]; then
	cat myconfig.xml | ./testunit -run 1>/dev/null
	curl https://raw.githubusercontent.com/hyperledger/fabric-samples/release-1.3/first-network/configtx.yaml | ./testunit -hype  1>/dev/null
fi

