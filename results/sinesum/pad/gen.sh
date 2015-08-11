#!/bin/sh

# iterate on devices
names=`ls -1 blocksize-*`
for name in ${names}; do

  basename=`basename ${name} .txt`
  title=`echo ${basename} | tr '-' ' '`
  size=`echo ${basename} | cut -d'-' -f2`

  ./sinesum.sh ${basename} "${title}" `python -c "print ${size}/31250.0*1000"`
  epspdf img/${basename}.eps img/${basename}.pdf
done

for i in "" "-for"; do 
  if [ -z "${i}" ]; then
    name="using inline code"
    key="above horizontal"
    height=0.61
  else
    name="usando um loop"
    key="off"
    height=0.495
    #height=0.61
  fi
  ./sinesum-comparison.sh "${i}" "${name}" "${key}" ${height}
  epspdf img/sinesum-comparison${i}.eps img/sinesum-comparison${i}.pdf
done
