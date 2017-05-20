# tp-2017-1c-The-Kernels

PROTOCOLO

UNIVERSAL:<br/>
0 - HANDSHAKE<br/>
-1 - DESCONEXION ABRUPTA<br/>
104 - OK (Si quieren, viene de 10-4 xdDdDDDDDDdddDdDD)<br/>

CONSOLA-KERNEL:<br/>
1 - INICIAR PROCESO<br/>
9 - MATAR PROCESO<br/>
-2: No hay espacio en memoria<br/>

KERNEL-MEMORIA:<br/>
1 - INICAR PROCESO<br/>
-2: No hay espacio en memoria<br/>

CPU-MEMORIA:<br/>
1 - CAMBIO DE PROCESO<br/>
2 - LEER<br/>
3 - ESCRIBIR<br/>
4 - DESCONEXION NORMAL DE CPU<br/>

CPU-KERNEL<br/>
1 - SE TERMINO LA EJECUCION DE UN PROCESO (NO HAY MAS INSTRUCCIONES)<br/>
