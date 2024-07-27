echo "converting..."
python3.10 mkpts.py $1
echo "finding grid..."
gcc grid.c; ./a.out
echo "combining..."
gcc combine.c; ./a.out
echo "faking veg..."
echo '#define CLS "veg"' > arg.h
gcc fake.c; ./a.out
echo "faking gnd..."
echo '#define CLS "gnd"' > arg.h
gcc fake.c; ./a.out
echo "cleaning..."
rm -rf data/*
echo "done"
