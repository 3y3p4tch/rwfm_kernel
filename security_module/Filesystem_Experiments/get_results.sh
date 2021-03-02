if [ $1 == "kernel" ]
then
    cd ..
    make
    cp system_call_interceptor.ko ./Filesystem_Experiments/
    cd Filesystem_Experiments
    sudo insmod system_call_interceptor.ko
fi

./run.sh
for CHUNK_SZ in 16 32 64
do
    echo "Chunk size = $CHUNK_SZ"
    ./create
    ./write_f $CHUNK_SZ
    ./read_write $CHUNK_SZ
    ./read_f $CHUNK_SZ
    rm -f temp_file
done
./clean.sh

if [ $1 == "kernel" ]
then
    sudo rmmod system_call_interceptor
fi
