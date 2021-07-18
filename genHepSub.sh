#!/bin/bash

tag=ggF

inpath=/publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/ntuple/h026/sys/

allJobs=jobsSub.sh
> ${allJobs}

intvl=9
for sysClass in jetsys jetsysdata photonallsys1 photonallsys2 photonsys;do
  for init in 1 11 21 31 41 51 61 71 81 91 101;do

   #fin=`expr ${init} + ${intvl}`
   fin=$((${init} + ${intvl}))
   jobName=${sysClass}_${init}_${fin}_${tag}; echo ${jobName}

   if [ ! -d hepsub_${jobName} ]; then mkdir hepsub_${jobName}; fi
   > exe_${jobName}.sh

   echo "#!/bin/bash" >> exe_${jobName}.sh
   echo "" >> exe_${jobName}.sh
   echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/reduceSize/sys" >> exe_${jobName}.sh
   echo "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase" >> exe_${jobName}.sh
   echo "source \${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh" >> exe_${jobName}.sh
   echo "lsetup \"root 6.20.06-x86_64-centos7-gcc8-opt\"" >> exe_${jobName}.sh
   echo "root -b -l -q reduceSysNtupleSize.cxx\(\\\"${sysClass}\\\",${init},${fin},\\\"${inpath}\\\"\)" >> exe_${jobName}.sh

   chmod +x exe_${jobName}.sh

   echo "hep_sub exe_${jobName}.sh -g atlas -os CentOS7 -wt mid -mem 2048 -o hepsub_${jobName}/log-0.out -e hepsub_${jobName}/log-0.err" >> ${allJobs}

   if [ "$(ls hepsub_${jobName}/)" != "" ];then rm hepsub_${jobName}/*;fi

  done
done

chmod +x ${allJobs}
