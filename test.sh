#! /bin/bash

cd ~/mpc/PSI5
make cleanall
make

for setLen in 256 4096 65536 1048576; do
#for setLen in 256; do
  echo "setLen: ${setLen}"

  rm -rf ~/mpc/PSI5/sample_sets/emails_alice.txt
  rm -rf ~/mpc/PSI5/sample_sets/emails_bob.txt
  rm -rf ~/mpc/PSI5/result/alice-${setLen}
  rm -rf ~/mpc/PSI5/result/bob-${setLen}

  cd ~/mpc/PSI5/sample_sets/emailgenerator
  ./emailgen.py ${setLen} >>~/mpc/PSI5/sample_sets/emails_alice.txt
  ./emailgen.py ${setLen} >>~/mpc/PSI5/sample_sets/emails_bob.txt
  #  ./emailgen.py 100 >> ~/mpc/PSI5/sample_sets/emails_bob.txt

  cd ~/mpc/PSI5
  gnome-terminal --title="alice" -x bash -c "./demo.exe -r 0 -p 3 -f sample_sets/emails_alice.txt -t >> ~/mpc/PSI5/result/alice-${setLen}"
  gnome-terminal --title="bob" -x bash -c "./demo.exe -r 1 -p 3 -f sample_sets/emails_bob.txt   -t >> ~/mpc/PSI5/result/bob-${setLen}"
#  gnome-terminal --title="alice" -x bash -c "./psi.exe -r 0 -p 3 -f sample_sets/emails_alice.txt -t >> ~/mpc/PSI5/result/alice-${setLen}"
#  gnome-terminal --title="bob" -x bash -c "./psi.exe -r 1 -p 3 -f sample_sets/emails_bob.txt   -t >> ~/mpc/PSI5/result/bob-${setLen}"

  sleep 1s
done
