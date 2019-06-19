mkdir /home/pi/nano;
cd /home/pi/nano;
npm install avrgirl-arduino;
wget -O smt172-needlevalve.hex https://raw.githubusercontent.com/nrbrt/smt172-nano-needlevalve/master/smt172-needlevalve.hex;
wget -O programnano.js https://raw.githubusercontent.com/nrbrt/smt172-nano-needlevalve/master/programnano.js;
node /home/pi/nano/programnano.js;
cd /home/pi;
rm -r /home/pi/nano;