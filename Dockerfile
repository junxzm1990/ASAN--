FROM yzhang71/asanopt:latest
RUN apt-get update && apt-get -y install cmake \
	clang-4.0 \
	tar \
	wget \
	sendmail \
	vim \
	git \
	pkg-config \
	fontconfig \
	libfontconfig1-dev
COPY testcases /home/testcases


