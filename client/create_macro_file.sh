macro_file_name="database_macros.h"
database_ops_file_name="database_operations.txt"
count=0

rm -f $macro_file_name
touch $macro_file_name

echo "#ifndef _DB_MACROS_H_" > $macro_file_name
echo "#define _DB_MACROS_H_" >> $macro_file_name
echo "" >> $macro_file_name
echo "#define SEMAPHORE_NAME \"/database_semaphore\"" >> $macro_file_name
echo "" >> $macro_file_name
echo "#define REQUEST_FIFO_PATH \"/tmp/request_fifo\"" >> $macro_file_name
echo "#define RESPONSE_FIFO_PATH \"/tmp/response_fifo\"" >> $macro_file_name
echo "" >> $macro_file_name
echo "#define REQUEST_DELIM \" \"" >> $macro_file_name
echo "" >> $macro_file_name
echo "#define MAX_REQUEST_LENGTH 1024" >> $macro_file_name

while IFS="" read -r line || [[ -n "$line" ]]; do
    if [[ $line =~ ^#.* ]] || [[ -z $line ]]
    then
        echo "" >> $macro_file_name
    else
        echo "#define $line $count" >> $macro_file_name
        ((count++))
    fi
done < "$database_ops_file_name"

echo "" >> $macro_file_name
echo "#endif" >> $macro_file_name
