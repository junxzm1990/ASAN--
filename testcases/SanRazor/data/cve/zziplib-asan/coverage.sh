#!/bin/bash
echo "
#include <cstdint>
#include <cstdio>
" > $1.cpp

cat >> $1.cpp << EOF
extern "C" {
#define CCOUNT(X) COUNTER_##X
EOF


echo "extern uint64_t CCOUNT(numSCBranches$1);">> $1.cpp
echo "extern uint64_t CCOUNT(numUCBranches$1);">> $1.cpp

cat >> $1.cpp << EOF
extern struct {
    uint64_t id;
    uint64_t count[3];
EOF

echo -n "} " >> $1.cpp

echo -n "CCOUNT(UCBranchInfo$1)[], ">> $1.cpp  
echo -n "CCOUNT(SCBranchInfo$1)[], ">> $1.cpp

echo "END;" >> $1.cpp

cat >> $1.cpp << EOF
struct BrInfo{
    uint64_t id;
    uint64_t count[3];
} ;
EOF


echo "void
CCOUNT(calledSC$1)(uint64_t index, uint64_t type) {
    ++CCOUNT(SCBranchInfo$1)[index].count[type];
}

void
CCOUNT(calledUC$1)(uint64_t index, bool cond) {
    ++CCOUNT(UCBranchInfo$1)[index].count[0];
    CCOUNT(UCBranchInfo$1)[index].id = index;
    if (cond) {
        ++CCOUNT(UCBranchInfo$1)[index].count[1];
    }
    else {
        ++CCOUNT(UCBranchInfo$1)[index].count[2];
    }
}" >> $1.cpp


echo "
void
CCOUNT(printSC$1)() {
    FILE *fp = fopen(\"$3$1_SC.txt\",\"rb\");
    if (fp == NULL) {
        fp = fopen(\"$3$1_SC.txt\",\"wb\");
        for (size_t id = 0; id < CCOUNT(numSCBranches$1); ++id) {
            auto info = CCOUNT(SCBranchInfo$1)[id];
            info.id = id;
            fwrite(&info,sizeof(info),1,fp);
        }
        fclose(fp);
    }
    else {
        for (size_t id = 0; id < CCOUNT(numSCBranches$1); ++id) {
            auto info = CCOUNT(SCBranchInfo$1)[id];
            fread(&info, sizeof(info), 1, fp);
            CCOUNT(SCBranchInfo$1)[id].count[0] += info.count[0];
            CCOUNT(SCBranchInfo$1)[id].count[1] += info.count[1];
            CCOUNT(SCBranchInfo$1)[id].count[2] += info.count[2];
        }
        fp = fopen(\"$3$1_SC.txt\",\"wb\");
        for (size_t id = 0; id < CCOUNT(numSCBranches$1); ++id) {
            auto info = CCOUNT(SCBranchInfo$1)[id];
            info.id = id;
            fwrite(&info,sizeof(info),1,fp);
        }
        fclose(fp);
    }
}

void
CCOUNT(printUC$1)() {
    FILE *fp = fopen(\"$3$1_UC.txt\",\"rb\");
    if (fp == NULL) {
        fp = fopen(\"$3$1_UC.txt\",\"wb\");
        for (size_t id = 0; id < CCOUNT(numUCBranches$1); ++id) {
            auto info = CCOUNT(UCBranchInfo$1)[id];
            info.id = id;
            fwrite(&info,sizeof(info),1,fp);
        }
        fclose(fp);
    }
    else {
        for (size_t id = 0; id < CCOUNT(numUCBranches$1); ++id) {
            auto info = CCOUNT(UCBranchInfo$1)[id];
            fread(&info, sizeof(info), 1, fp);
            CCOUNT(UCBranchInfo$1)[id].count[0] += info.count[0];
            CCOUNT(UCBranchInfo$1)[id].count[1] += info.count[1];
            CCOUNT(UCBranchInfo$1)[id].count[2] += info.count[2];
        }
        fp = fopen(\"$3$1_UC.txt\",\"wb\");
        for (size_t id = 0; id < CCOUNT(numUCBranches$1); ++id) {
            auto info = CCOUNT(UCBranchInfo$1)[id];
            info.id = id;
            fwrite(&info,sizeof(info),1,fp);
        }
        fclose(fp);
    }
}" >> $1.cpp


echo "}" >> $1.cpp



clang++ -c $1.cpp $4 -o $2
