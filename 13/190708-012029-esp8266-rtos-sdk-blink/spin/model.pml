mtype = {APAGADO, ENCENDIDO, DESCARMADO, ARMADO, MAIN}

byte estado

/* Luz */
bit boton;
bit timeout;
bit luz;

/* Alarma */
bit codigo_valido;
bit presencia;
bit alarma;

/* Code */
bit botonCodigo;
bit incrementarCodigoActual;
bit tiempo;
bit codigoCorrecto;
bit siguienteDigito;
bit reset;


// #######################      ltl luz
ltl spec_luz_1 {
	[] (((state == APAGADO) && boton) -> <> (state == ENCENDIDO))
}

ltl spec_luz_2 {
	[] (((state == ENCENDIDO) && button) -> <> (state == ENCENDIDO))
}

ltl spec_luz_3 {
	[] (((state == ENCENDIDO) && timeout) -> <> (state == APAGADO))
}

ltl spec_luz_4 {
	[] (((state == APAGADO) && boton) -> <> (luz == 1))
}

ltl spec_luz_5 {
	[] (((state == ENCENDIDO) && button) -> <> (luz == 0))
}



// #######################      ltl alarma
ltl spec_alar_1 {
	[](((state == DESCARMADO) && codigo_valido) -> <> (state == ARMADO))
}

ltl spec_alar_2 {
	[](((state == ARMADO) && presencia) -> <> (state == ARMADO))
}

ltl spec_alar_3 {
	[](((state == ARMADO) && codigo_valido) -> <> (state == DESCARMADO))
}

ltl spec_alar_4 {
	[](((state == DESCARMADO) && codigo_valido) -> <> (alarma == 0))
}

ltl spec_alar_5 {
	[](((state == ARMADO) && presencia) -> <> (alarma == 1))
}

ltl spec_alar_6 {
	[](((state == ARMADO) && (presencia == 0)) -> <> (alarma == 0))
}

// #######################      ltl codigo
ltl spec_code_1 {
	[] (butonCodigo -> <> (incrementarCodigoActual == 1))
}

ltl spec_code_2 {
	[] (tiempo && codigoCorrecto -> <> (siguienteDigito == 1))
}

ltl spec_code_3 {
	[] ((tiempo && (codigoCorrecto == 0)) -> <> (reset == 1))
}




/* Promela */

active proctype luz_fsm () {
    state = APAGADO;
    do
    :: (state == APAGADO) -> atomic {
        if
        :: (boton == 1) -> state = ENCENDIDO; luz = 1; boton = 0
        fi
    }
    :: (state == ENCENDIDO)  -> atomic {
        if
        :: (boton == 1) -> state = ENCENDIDO; luz = 1; boton = 0
        :: (timeout == 1)-> state = APAGADO; luz = 0; timeout = 0
        fi
    }
    od
}

active proctype alarma_fsm (){
	state = DESCARMADO
	do
    :: (state == DESCARMADO) -> atomic {
        if
        :: codigo_valido -> state = ARMADO; codigo_valido = 0; alarma = 0;
        fi
    }
    :: (state == ARMADO)  -> atomic {
        if
        :: codigo_valido -> state = DESCARMADO; codigo_valido = 0; alarma = 0;
        :: (!codigo_valido && presencia) -> alarma = 1; presencia = 0
        fi
    }
    od
}

active proctype codigo_fsm (){
	state = MAIN
	do
    :: (state == MAIN) -> atomic {
        if
        :: botonCodigo -> incrementarCodigoActual = 1; botonCodigo = 0; 
        :: (tiempo && codigoCorrecto) -> siguienteDigito = 1; tiempo = 0; codigoCorrecto = 0;
        :: (tiempo && !codigoCorrecto) -> reset = 1; tiempo = 0;
        fi
    }
    od
}

active proctype entorno () {
	do
    :: timeout = 0
    :: tiempo = 0;
    :: boton = 1;
    :: botonCodigo = 0;
    :: presencia = 0;
    :: codigoCorrecto = 0;
    :: skip
    od
}