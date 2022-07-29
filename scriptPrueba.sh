cd /home/utnso
rm -rf so-commons-library

git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library
make install

cd /home/utnso/tp-2022-1c-PaguenNuestroPsicologo/psicoLibrary/Debug
make clean all
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2022-1c-PaguenNuestroPsicologo/psicoLibrary/Debug

cd /home/utnso/tp-2022-1c-PaguenNuestroPsicologo/Consola/Debug
make clean all

cd /home/utnso/tp-2022-1c-PaguenNuestroPsicologo/Kernel/Debug
make clean all

cd /home/utnso/tp-2022-1c-PaguenNuestroPsicologo/MemoriaSwap/Debug
make clean all

cd /home/utnso/tp-2022-1c-PaguenNuestroPsicologo/CPU/Debug
make clean all

cd /home/utnso/tp-2022-1c-PaguenNuestroPsicologo