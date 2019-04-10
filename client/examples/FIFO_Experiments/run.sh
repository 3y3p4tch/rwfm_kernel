rm tmp_fifo
gcc fifo_reader.c -o fifo_reader
gcc fifo_writer.c -o fifo_writer
mkfifo "tmp_fifo"
chmod 666 tmp_fifo
