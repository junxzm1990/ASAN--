wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/imagetestsuite/imagetestsuite-tiff-1.00-part1.tar.gz
tar -xzf imagetestsuite-tiff-1.00-part1.tar.gz
mv tif tif1

wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/imagetestsuite/imagetestsuite-tiff-1.00-part2.tar.gz
tar -xzf imagetestsuite-tiff-1.00-part2.tar.gz
mv tif tif2

wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/imagetestsuite/imagetestsuite-tiff-1.00-part3.tar.gz
tar -xzf imagetestsuite-tiff-1.00-part3.tar.gz
mv tif tif3

wget https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/imagetestsuite/imagetestsuite-tiff-1.00-part4.tar.gz
tar -xzf imagetestsuite-tiff-1.00-part4.tar.gz
mv tif tif4

mkdir tif
cp tif1/* ./tif
cp tif2/* ./tif
cp tif3/* ./tif
cp tif4/* ./tif

rm -rf tif1 tif2 tif3 tif4