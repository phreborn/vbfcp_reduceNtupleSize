#!/bin/bash

opt=$1

for f in `ls mc16*/*/*Nominal*root`
do
  if [[ ! "${f}" =~ "photonsys" && ! "${f}" =~ "photonallsys1" && ! "${f}" =~ "jetsysdata" ]];then rm ${f};fi
done

basepath=$(pwd)
targetpath="/scratchfs/atlas/huirun/atlaswork/VBF_CP/ntuples/sys/"
targetpathjd="/scratchfs/atlas/huirun/atlaswork/VBF_CP/ntuples/sys/jetsysdata/"

declare -A dsid=(["ggF"]="343981" ["VBF"]="346214")
echo ${!dsid[*]}

> cmd_tmp
echo "#!/bin/bash" >> cmd_tmp
echo "" >> cmd_tmp

for camp in mc16a mc16d mc16e;
do
  #cd ${basepath}/${camp}
  #pwd
  for proc in ggF VBF;
  do
    targety=${targetpath}/yield/${camp}/${dsid[${proc}]}_${proc}_allSys.root
    targetjd=${targetpathjd}/${camp}/${dsid[${proc}]}_${proc}_jetsysdata.root
    targets=${targetpath}/shape/${camp}/${dsid[${proc}]}_${proc}_allSys.root

    cmdy="if [ -f ${targety} ];then rm ${targety};fi; hadd ${targety} ${camp}/${proc}_jetsys/*root ${camp}/${proc}_photonsys/*root"
    cmdjd="if [ -f ${targetjd} ];then rm ${targetjd};fi; hadd ${targetjd} ${camp}/${proc}_jetsysdata/*root"
    cmds="if [ -f ${targets} ];then rm ${targets};fi; hadd ${targets} ${camp}/${proc}_photonallsys1/*root ${camp}/${proc}_photonallsys2/*root"

    #echo ${cmdy}
    #echo ${cmds}
    echo "${cmdy}" >> cmd_tmp
    echo "${cmdjd}" >> cmd_tmp
    echo "${cmds}" >> cmd_tmp
    echo "" >> cmd_tmp
  done
  echo ""
done

cat cmd_tmp
if [ "${opt}" = "exe" ];then
  chmod +x cmd_tmp
  source cmd_tmp
fi
rm cmd_tmp

#cd ${basepath}
