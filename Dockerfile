FROM yzhang71/asanopt:latest
RUN apt-get update && apt-get -y install cmake \
	clang-4.0 \
	tar \
	wget \
	sendmail \
	vim
COPY testcases /home/testcases

