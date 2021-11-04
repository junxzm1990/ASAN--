git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PATH:/home/testcases/chromium/depot_tools"
mkdir /home/testcases/chromium/chromium && cd /home/testcases/chromium/chromium
fetch --nohooks chromium

cd /home/testcases/chromium/chromium/src && git checkout tags/58.0.3003.0 -b 58
COMMIT_DATE=$(git log -n 1 --pretty=format:%ci)
echo $COMMIT_DATE

cd /home/testcases/chromium/depot_tools && git checkout $(git rev-list -n 1 --before="$COMMIT_DATE" main)
export DEPOT_TOOLS_UPDATE=0

cd /home/testcases/chromium/chromium/src
export DEPOT_TOOLS_UPDATE=0
gclient sync -D --force --reset --with_branch_heads

cd /home/testcases/chromium/chromium/src
patch -p1 < /home/testcases/chromium/chromium_patch

add-apt-repository ppa:ubuntu-toolchain-r/test
apt-get update
apt-get -y install gcc-4.9
apt-get -y upgrade libstdc++6