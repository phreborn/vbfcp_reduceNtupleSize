tag=ggF

allJobs=jobsSub.sh
> ${allJobs}

intvl=9
for sysClass in jetsys photonallsys1 photonallsys2 photonsys;do
  for init in 1 11 21 31 41 51 61 71 81 91 101;do

   #fin=`expr ${init} + ${intvl}`
   fin=$((${init} + ${intvl}))
   jobName=${sysClass}_${init}_${fin}_${tag}; echo ${jobName}

   if [ ! -d submit_${jobName} ]; then mkdir submit_${jobName}; fi
   > exe_${jobName}.sh

   echo "#!/bin/bash" >> exe_${jobName}.sh
   echo "" >> exe_${jobName}.sh
   echo "cd /afs/cern.ch/work/h/huirun/workspace/vbf_cp/reduceSize/sys" >> exe_${jobName}.sh
   echo "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase" >> exe_${jobName}.sh
   echo "source \${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh" >> exe_${jobName}.sh
   echo "lsetup \"root 6.20.06-x86_64-centos7-gcc8-opt\"" >> exe_${jobName}.sh
   echo "root -b -l -q reduceSysNtupleSize.cxx\(\\\"${sysClass}\\\",${init},${fin}\)" >> exe_${jobName}.sh

   cat example.sub | sed 's/??/'${jobName}'/g' > ${jobName}.sub

   echo "condor_submit ${jobName}.sub" >> ${allJobs}

  done
done

chmod +x ${allJobs}
