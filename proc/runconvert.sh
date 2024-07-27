HERE=$(pwd)
cd ../erikstrand/galiano
source env/bin/activate
cd galiano
# curl $2 > data/laz/$1.laz
cp $HERE/dl/$1.laz data/laz/$1.laz
make all
mv data/npz/$1_xyz.npz $HERE/data/$1_xyz.npz
mv data/npz/$1_class.npz $HERE/data/$1_class.npz
mv data/npz/$1_lat.npz $HERE/data/$1_lat.npz