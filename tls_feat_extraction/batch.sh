folder1="../dataset/android" 

files=$(ls $folder1)
for file in $files;
do
    file_path=$folder1"/"$file
    ./Bin/TLSFingerprinting -r $file_path
done

