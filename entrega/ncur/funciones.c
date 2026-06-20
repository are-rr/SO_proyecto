else if (strcmp(comando, "mata") == 0)
{
    int mato_ejecucion = procesarMata(&lista_ejecucion, &lista_terminados, &lista_listos, &lista_suspendidos, &lista_nuevos, num_palabras, archivo, swap, RAM, TMM, TMS, &grupos, &porS, &porR);
    if (mato_ejecucion)
    {
        break;
    }
    continue;
    /*if (num_palabras < 2)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: falta el PID del proceso");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        comando[0] = '\0';
        num_PID = '\0';
        num_palabras = 0;
        continue;
    }
    num_PID = atoi(archivo);
    if (!num_PID)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: PID debe ser un entero");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        continue;
    }
    if (num_palabras > 2)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        comando[0] = '\0';
        num_PID = '\0';
        num_palabras = 0;
        continue;
    }

    refresh();
    int gid_matado = -1;
    int lista = 0;
    struct Nodo *p_matar = buscar(lista_ejecucion, num_PID);

    if (p_matar == NULL){
        p_matar = buscar(lista_listos, num_PID);
    }else if(p_matar !=NULL){
        lista = 1;//Esta en lista LISTOS
    }
    if (p_matar == NULL){
        p_matar = buscar(lista_suspendidos, num_PID);
    }
    if (p_matar == NULL){
        p_matar = buscar(lista_nuevos, num_PID);
        lista = 2;//esta en lista NUEVOS
    }
    if (p_matar != NULL){
        gid_matado = p_matar->GID;
    }
    mvprintw(50,0,"PID_matar: %d",num_PID);

    refresh();
    napms(2000);

    struct Nodo *pMata = matar(&lista_ejecucion, &lista_terminados, &lista_listos, &lista_suspendidos, &lista_nuevos, num_PID);
    if (pMata == NULL){
        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados,lista_suspendidos, lista_nuevos);
        continue;
    }
    if (gid_matado != -1 && pMata != NULL)
    {
        if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, pMata->GID) == 0 && lista != 2)
        {

            liberarSWAP(swap, pMata->TMP, pMata->num_paginas, TMS);
            liberarRAMproceso(RAM, TMM, pMata->PID);
            limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
            RevisarNuevos(&lista_nuevos,&lista_listos,swap,TMS);
            limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
            imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
            limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
            imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
            porcentajes(TMS, TMM, &porS, &porR);

            grupos--;
        }
        if(lista == 2){
            grupos--;
        }
    }

    mvprintw(y_variable,0,"numero de grupos:%d",grupos);
    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
    limpiarZona(y_linea_comando, 0, ancho_procesos);
    refresh();
    num_palabras = 0;
    if (lista == 1){ // 1 -> esta en lista ejecucion
        break;
    }

    continue;*/
}

else if (strcmp(comando, "fork") == 0)
{
    procesarFork(&lista_ejecucion, &lista_terminados, &lista_listos, &lista_suspendidos, num_palabras, archivo, extra, &pid);

    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);

    limpiarZona(y_linea_comando, 0, ancho_procesos);
    refresh();

    continue;
    /*if (num_palabras < 2)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: falta pid del proceso");
        refresh();
        num_palabras = 0;
        continue;
    }
    if (num_palabras < 3)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: falta numero de instruccion");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        num_palabras = 0;
        continue;
    }
    if (Negativo(extra) == 1){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "Error: PC no puede ser negativo");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        continue;
    }
    num_PID = atoi(archivo); //NOTA:no borrar si se usa, es lo que mandamos a la funcion
    num_PC = atoi(extra);
    if (!Digito(archivo))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: PID debe ser un entero");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        num_PID = '\0';
        num_palabras = 0;
        num_PID = '\0';
        continue;
    }
    if (!Digito(extra)){ // NOTA: atoi no filta bien el termino ya que acepta fork 1 2gakigski
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: PC debe ser un entero");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        num_palabras = 0;
        num_PID = '\0';
        num_PC = '\0';
        continue;
    }
    if (num_palabras > 3)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
        limpiarZona(y_linea_comando, 0, ancho_procesos);
        refresh();
        num_palabras = 0;
        num_PID = '\0';
        num_PC = '\0';
        continue;
    }
    pid++;
    // num_PID num_PC son las variables que estan en el comando
    struct Nodo *nuevo = forkProcesoComando(&lista_ejecucion, &lista_terminados, &lista_listos, &lista_suspendidos, num_PID, num_PC, pid);

    if (nuevo == NULL)
    {
        pid--;
    }

    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
    limpiarZona(y_linea_comando, 0, ancho_procesos);
    refresh();*/
}