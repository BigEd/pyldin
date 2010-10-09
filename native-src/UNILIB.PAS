 (************************************************************)
(*							    *)
(*	      П░ог░амма ▒оздани┐ и ко░░ек▓и░овки	    *)
(*		  библио▓еки об'ек▓н╗╡ мод│лей              *)
(*			 Version 1.02			    *)
(*		  (c) 19.VII.1989 Leonid Curak		    *)
(*							    *)
(************************************************************)

 PROGRAM LIBR;
(*
	 Опи▒ание ▓ипов и пе░еменн╗╡
*)

 CONST MAXD=10000;	 (*  Коли╖е▒▓во ╜лемен▓ов в ░або╖ем ма▒▒иве *)

 TYPE	ARBF = array[1..MAXD] OF BYTE;
	ARB  = ARRAY[1..8] OF BYTE;
	tmod = record
	    kom      : byte;
	    nam_mod  : string[40];
	    name_mod : arb;
	  end;
	tmk  = array[1..32] of tmod;

 VAR IP,I,IC,KMOD,KMODN,DLM,DLS,IND	 :INTEGER;
     IT,IM,j,jj,kmm 			 :INTEGER;
     FLPR,FLIG,FS			 :BOOLEAN;
     CH 				 :CHAR;
     BUF				 :ARBF;
     LIB_NAM,LIB_NMP,NAM_PRN,PAR 	 :STRING[80];
     STRN				 :STRING[80];
     KD,koma 				 :BYTE;
     NAME_MODT				 :ARB;
     RFILE,MFILE,WFILE			 :FILE; 	(* FILES *)
     PFILE				 :TEXT;
     DLF				 :LONGINT;
     digits				 : array[0..15] of char;
     moda				 : tmk;

(*
		   Подп░ог░амм╗
*)
  FUNCTION PRHEX(I:  BYTE) : STRING;

  BEGIN
    PRHEX := DIGITS[I DIV 16] +  DIGITS[I MOD 16];
  END; { PRHEX }


(*
     П░о╢ед│░а в╗делени┐ имени мод│л┐ из имени ┤айла
*)
   procedure MODNAM(nam:string; var mnmd:arb);

    var j,ii,k,kk : integer;
	ch	  : char;

    begin

     ii:=1;
     k:=LENGTH(nam);		{ длина ▒▓░оки }
     j:=1;

     repeat
       ch:=nam[ii];
       ii:=ii+1;
       kk:=POS(ch,':\.');       { один из ▒пе╢. ▒имволов? }
       if kk=0 then
	 begin
	  mnmd[j]:=ORD(UPCASE(ch));  { пе░екоди░овка в бол╝╕ие б│кв╗ }
	  j:=j+1;
	 end
	else
	 if kk<>3 then j:=1;
     until (kk=3) or (ii>k);
     if j<16 then		 { дополнение п░обелами }
       for k:=j to 16 do
	 mnmd[k]:=$20
   end; 	       { PROC MODNAM }

(*
       Об░або▓ка одного мод│л┐

  В╡од:    FRD	- ┤айл ╖▓ени┐
	   FWR	- ┤айл запи▒и
	   FZP	- ┤лаг запи▒и
	   FPR	- ┤лаг пе╖а▓и
*)
 PROCEDURE ZMODA(VAR FRD,FWR:FILE;NAM_T: ARB; FZP,FPR:BOOLEAN);
  VAR  KPB,KEXT,DLPB,DLEXT,DL:INTEGER;
 begin

  STRN:=' ';
  FOR I:=1 TO 8 DO
    BEGIN
      CH:=CHR(NAM_T[I]);
      STRN:=STRN+CH;
    END;
  IF FPR THEN	       (* Пе╖а▓╝ заголовка мод│л┐ *)
(*
	      БЛОК ПЕЧАТИ имени мод│л┐
*)
     WRITELN(PFILE,'   МОДУЛЬ',STRN);

  BLOCKREAD(FRD,BUF,32);	   (*  Ч▓ение ╕апки мод│л┐*)
  IF (BUF[1]<>$5A) OR (BUF[2]<>$A5) THEN
    WRITELN(' LIB  Мод│л╝ ',strn,' не об║ек▓н╗й')
   ELSE
    BEGIN
     IF FZP THEN BLOCKWRITE(FWR,BUF,32);  (*  Запи▒╝ заголовка мод│л┐ *)
     KPB:=BUF[5]*256+BUF[6];	    (*	Коли╖е▒▓во ▓о╖ек PUBLIC *)
     KEXT:=BUF[7]*256+BUF[8];	    (*	Коли╖е▒▓во ▓о╖ек EXT	*)
     DLPB:=KPB*20;		    (*	Коли╖е▒▓во бай▓ в PUBLIC*)
     DLEXT:=KEXT*20;		    (*	Коли╖е▒▓во бай▓ в EXT	*)
     BLOCKREAD(FRD,BUF,DLPB);		 (*  Ч▓ение обла▒▓и PUBLIC   *)
     IF DLPB<>0 THEN
      BEGIN
       IF FZP THEN BLOCKWRITE(FWR,BUF,DLPB); (*  Запи▒╝ обла▒▓и PUBLIC	 *)
       IF FPR THEN	    (* Пе╖а▓╝ ▓абли╢╗ в╡одн╗╡ ▓о╖ек *)
	BEGIN
	 WRITELN(PFILE,' ');
	 WRITELN(PFILE,'      ТАБЛИЦА ВХОДНЫХ ТОЧЕК ');
	 WRITELN(PFILE,' ');
	 IND:=0;
	 FOR I:=1 TO KPB DO
	  BEGIN
	   STRN:=' ';
	   FOR IP:=1 TO 16 DO
	    BEGIN
	     STRN:=STRN+CHR(BUF[IND+IP]);
(*	       STRN:=CONCAT(STRN,CH);*)
	    END;
	   STRN:=STRN+'  ';
	   IF BUF[IND+19]=0 THEN CH:='R' ELSE CH:='C';
	   STRN:=STRN+CH+' ';
	   STRN:=STRN+PRHEX(BUF[IND+17]);
	   STRN:=STRN+PRHEX(BUF[IND+18]);
	   WRITELN(PFILE,STRN);
	   IND:=IND+20;
	  END;
	  WRITELN(PFILE);
	END
      END;
     IF DLEXT<>0 THEN
      BEGIN
       BLOCKREAD(FRD,BUF,DLEXT);	   (*  Ч▓ение обла▒▓и EXT   *)
       IF FZP THEN BLOCKWRITE(FWR,BUF,DLEXT); (*  Запи▒╝ обла▒▓и EXT   *)
       IF FPR THEN	    (* Пе╖а▓╝ ▓абли╢╗ вне╕ни╡ ▓о╖ек *)
	BEGIN
	 WRITELN(PFILE);
	 WRITELN(PFILE,'      ТАБЛИЦА ВНЕШНИХ ТОЧЕК ');
	 WRITELN(PFILE);
	 IND:=0;
	 FOR I:=1 TO KEXT DO
	  BEGIN
	   STRN:=' ';
	   FOR IP:=1 TO 16 DO
	    BEGIN
	     STRN:=STRN+CHR(BUF[IND+IP]);
{	      STRN:=CONCAT(STRN,CH);}
	    END;
	    WRITELN(PFILE,STRN);
	    IND:=IND+20;
	  END;
	END;
      END;
     DLF:=DLF-DLPB-DLEXT-32;	   (* О▒▓а▓ок ┤айла в бай▓а╡ *)
(*
	     Пе░ека╖ка ▓ела мод│л┐
*)
     REPEAT
       IF DLF<MAXD THEN        (*  В╗╖и▒ление длин╗ о╖е░едного мод│л┐ *)
	 DL:=DLF
	ELSE
	 DL:=MAXD;
       DLF:=DLF-DL;
       BLOCKREAD(FRD,BUF,DL);
       IF FZP THEN BLOCKWRITE(FWR,BUF,DL);
     UNTIL DLF=0
    END
  END;	      (*  END  ZMODA *)

(*
      ПРОЦЕДУРА ЧТЕНИЯ ДЛИНЫ МОДУЛЯ
*)
   PROCEDURE RDDL(VAR FRD:FILE);
    BEGIN
     DLF:=0;
     BLOCKREAD(FRD,BUF,4);
     FOR IT:=1 TO 4 DO
      BEGIN
       DLF:=DLF*256+BUF[IT];
      END;
    END;

(*
     ПРОЦЕДУРА ПЕЧАТИ ЗАГОЛОВКА БИБЛИОТЕКИ
*)
  PROCEDURE PRZAG;
   BEGIN
    WRITELN(PFILE,'            БИБЛИОТЕКА ',LIB_NAM);
    WRITELN(PFILE);
    WRITELN(PFILE);
    WRITELN(PFILE,'   В БИБЛИОТЕКЕ ',KMODN,' МОДУЛЕЙ');
   END;

(*
      ПРОЦЕДУРА ПРЕОБРАЗОВАНИЯ ДЛИНЫ
*)
   PROCEDURE DLINA(VAR FRD:FILE);
    VAR DLF1	     :LONGINT;
    BEGIN
     DLF:=FILESIZE(FRD);
     DLF1:=DLF;
     FOR IT:=1 TO 4 DO
      BEGIN
       BUF[5-IT]:=DLF1 MOD 256;
       DLF1:=DLF1 DIV 256;
      END;
     END;


 PROCEDURE ERRPAR;
 BEGIN
 WRITELN('ERROR in Parameters');
 writeln('usage: UniLIB LibName [command] [/ListFile]');
 writeln('  command: <symbol>ModuleName, where symbol is');
 writeln('             + add ModuleName to the library');
 writeln('             - remove ModuleName from the library');
 writeln('             * extract ModuleName without removing it');
 writeln('             -+ or +- replace ModuleName in library');
 writeln('             -* or *- extract ModuleName and remove it');
 HALT;
 END;

(*
     Блок ░азбо░а па░аме▓░ов
*)
 BEGIN
  writeln('UniLIB Version 1.00. (c) 1989 "ТРИАДА".');

  digits:= '0123456789ABCDEF';
  IP:=PARAMCOUNT;	     (*    Коли╖е▒▓во па░аме▓░ов  *)
  IF IP<1 THEN ERRPAR;	      (*    О╕ибка в па░аме▓░а╡    *)
  LIB_NAM:=PARAMSTR(1);        (* Пе░в╗й па░аме▓░ (им┐ LIB) *)
  I:=POS('.',LIB_NAM);         (*   Номе░ ▓о╖ки в ▒▓░оке    *)
  IF I=0 THEN BEGIN	       (*  Им┐ без ░а▒╕и░ени┐	    *)
    LIB_NAM:=LIB_NAM+'.LIB';   (* Под▒оедини▓╝ ░а▒╕и░ение*)
    I:=POS('.',LIB_NAM)         (*   Номе░ ▓о╖ки в ▒▓░оке    *)
  END;
(*
     Пол│╖и▓╝ имена п░омеж│▓о╖ного ┤айла и ┤айла ▓ипа BAC
*)
  LIB_NMP:=COPY(LIB_NAM,1,I);
  LIB_NMP:=LIB_NMP+'$LB';       (* п░омеж│▓о╖н╗й ┤айл *)
  FLPR:=FALSE;			  (* Флаг пе╖а▓и *)
  j:=0;
  kmm:=0;
  koma:= 0;
(*
      Разбо░ о▒▓ал╝н╗╡ па░аме▓░ов
*)
  FOR I:=2 TO IP DO BEGIN
    PAR:=PARAMSTR(I);		   (* О╖е░едной па░аме▓░ *)
    CH:=PAR[1]; 	     (* CH=COPY(PAR,1,1) - пе░в╗й ▒имвол *)
(*
       П░ове░ка на команд│ и пе░екоди░овка: 1-*, 2-+,4--.
*)
    IC:=POS(CH,'*+ -/');
    IF IC=0 THEN ERRPAR;	 (* О╕ибка в па░аме▓░а╡ *)
    IF IC=5 THEN BEGIN		  (* Доп.команда: /P - пе╖а▓╝*)
      NAM_PRN:= PAR;
      DELETE(NAM_PRN,1,1);
      FLPR:=TRUE
      END
     ELSE BEGIN 		   (* Команда ░або▓╗ ▒ мод│лем *)
      j:=j+1;			   { номе░ па░аме▓░а }
      moda[j].kom:=IC;			   (* Код команд╗ *)
      IT:=1;			   (* Коли╖е▒▓во ▒имволов в команде *)
      CH:=PAR[2];	       (* CH=COPY(PAR,2,1) - в▓о░ой ▒имвол *)
      IC:=POS(CH,'*+ -');      (* П░ове░ка и пе░екоди░овка команд╗ *)
      IF IC=3 THEN ERRPAR;
      IF IC<>0 THEN BEGIN	(* П░одолжение команд╗ *)
	IT:=2;
	moda[j].kom:=moda[j].kom+IC;		(* Код команд╗ *)
	IF moda[j].KOM=3 THEN ERRPAR    (* +* - недоп│▒▓има┐ команда *)
      END;
      IF moda[j].KOM=2 THEN 		 (* Команда в▒▓авки + *)
        kmm:=kmm+1		 (* HOBOE *)
       ELSE
        IF (moda[j].KOM=4) OR (moda[j].KOM=5) THEN	 (* Команда │далени┐ *)
          kmm:=kmm-1;		 (* HOBOE *)
      koma:=koma or moda[j].kom;
      DELETE(par,1,IT); (* О▒▓ави▓╝ ▓ол╝ко им мод│л┐ *)
      IF POS('.',par) = 0 THEN
      	par:=par+'.OBJ';    (* Под▒оедини▓╝ ░а▒╕и░ение *)
      moda[j].NAM_MOD:= PAR;
(*
	Пол│╖и▓╝ им┐ мод│л┐ в ма▒▒иве бай▓ - NAME_MOD
*)
      FOR IP:=1 TO 8 DO
	moda[j].NAME_MOD[IP]:=$20;
      MODNAM(par,moda[j].NAME_MOD);
    END       (*  IF *)
  END;	      (* FOR *)
  FLIG:= (koma >= 2) and (j <>0);  (* Флаг ░едак▓и░овани┐ библио▓еки*)
(*
	 Блок на╖ал╝ной │▒▓ановки и о▓к░╗▓и┐ ┤айлов
*)
  IF NOT FLPR THEN NAM_PRN:='CON';
  FLPR:=TRUE;
  ASSIGN(PFILE,NAM_PRN);		(* OPEN FILE PRINT *)
  REWRITE(PFILE);
  IF IORESULT<>0 THEN BEGIN	(*  о╕ибка *)
    WRITELN(' ERROR DISK ',NAM_PRN);
    WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
    HALT(1)
   END;

  ASSIGN(RFILE,LIB_NAM);
  {$I-}
  RESET(RFILE,1);	  (* О▓к░╗▓╝ ┤айл библио▓еки *)
  IF IORESULT<>0 THEN BEGIN	(* Не▓ библио▓еки или о╕ибка *)
    IF koma <> 2  THEN errpar;
{      WRITELN(' ERROR DISK ',LIB_NAM);
      WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
      HALT(1)
    END; }
    ASSIGN(WFILE,LIB_NAM);
    REWRITE(WFILE,1);
    IF IORESULT<>0 THEN BEGIN	  (*  о╕ибка *)
      WRITELN(' ERROR DISK ',LIB_NAM);
      CLOSE(PFILE);
      WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
      HALT(1)
    END;

(*
       Создание новой библио▓еки
*)
    KMODN:=kmm;
    PRZAG;
    BUF[1]:=$0B;		  (* Создание ┤апки библио▓еки *)
    BUF[2]:=$B0;
    FOR I:=3 TO 32 DO  BUF[I]:=0;
    BUF[6]:=kmm;			 (* Коли╖е▒▓во мод│лей *)
    BLOCKWRITE(WFILE,BUF,32);	      (* Запи▒╝ заголовка библио▓еки *)

    for jj:=1 to j do
      begin

       ASSIGN(MFILE,moda[jj].NAM_MOD);
       RESET(MFILE,1);		  (* О▓к░╗▓╝ ┤айл мод│л┐ *)
       IF IORESULT<>0 THEN BEGIN	  (*  о╕ибка *)
         WRITELN(' ERROR DISK ',moda[jj].NAM_MOD);
         WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
         CLOSE(WFILE);
         ERASE(WFILE);
         CLOSE(PFILE);
         HALT(1);
       END;
  {$I+}
       BLOCKWRITE(WFILE,moda[jj].NAME_MOD,8);      (* Заголовок мод│л┐ - им┐ мод│л┐ *)
(*
	Пол│╖и▓╝ длин│ ┤айла MFILE и запи▒а▓╝ в BUF[1..4]
*)
       DLINA(MFILE);
       BLOCKWRITE(WFILE,BUF,4);	     (* Заголовок мод│л┐ - длина *)
       ZMODA(MFILE,WFILE,moda[jj].NAME_MOD,TRUE,FLPR); (* Пе░епи▒а▓╝ мод│л╝ *)
       CLOSE(MFILE);
      end;
    CLOSE(WFILE);
    CLOSE(PFILE);
    WRITELN(' LIB  КОНЕЦ РАБОТЫ');
    HALT(0)		  (* HOPMA *)
  END;
(*
       Рабо▓а ▒ библио▓екой
*)

  BLOCKREAD(RFILE,BUF,32);		  (* П░о╖и▓а▓╝ заголовок библио▓еки *)
  IF (BUF[1]<>$0B) OR (BUF[2]<>$B0) THEN BEGIN
    WRITELN('ФАЙЛ - ',LIB_NAM,' НЕ БИБЛИОТЕКА');
    CLOSE (RFILE);
    CLOSE (PFILE);
    WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
    HALT(2)
  END;
  if flig then
   begin
    ASSIGN(WFILE,LIB_NMP);
    REWRITE(WFILE,1);
   end;
  KMOD:=BUF[5]*256+BUF[6];	 (* Коли╖е▒▓во мод│лей в библио▓еке *)
  kmodn:=kmod+kmm;
  PRZAG;
  BUF[5]:=KMODN DIV 256;
  BUF[6]:=KMODN MOD 256;
  IF FLIG THEN BLOCKWRITE(WFILE,BUF,32);	 (* Запи▒╝ заголовка библио▓еки *)
(*
	Цикл по мод│л┐м библио▓еки (пои▒к заданного имени)
*)
  FOR IM:=1 TO KMOD DO
  BEGIN
    BLOCKREAD (RFILE,NAME_MODT,8);	(* Им┐ о╖е░едного мод│л┐ *)
    if j=0 then fs:=false
     else
      begin
       jj:=0;
       repeat
	 jj:=jj+1;

	 i:=1;
	 repeat 	  (* Цикл ▒░авнени┐ *)
	   fs:= moda[jj].NAME_MOD[I]=NAME_MODT[I];
	   i:=i+1;
	 until (i>8) or not fs;

       until (jj=j) or fs;
      end;
    IF FS THEN BEGIN		  (* На╕ли заданн╗й мод│л╝ *)
      CASE moda[jj].KOM OF
       1:			 (* Команда в╗бо░а '*'   *)
	BEGIN
	 ASSIGN(MFILE,moda[jj].NAM_MOD);
	 REWRITE(MFILE,1);  (* Созда▓╝ ┤айл мод│л┐ *)
	 IF IORESULT<>0 THEN BEGIN     (*  о╕ибка *)
	   CLOSE (RFILE);
	   CLOSE (PFILE);
	   WRITELN(' ERROR DISK ',moda[jj].NAM_MOD);
	   WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
	   HALT(1)
	 END;
	 RDDL(RFILE);	     (* П░о╖и▓а▓╝ и п░еоб░азова▓╝ длин│ мод│л┐ *)
	 ZMODA(RFILE,MFILE,NAME_MODT,TRUE,FLPR); (* Запи▒╝ мод│л┐ *)
	 CLOSE(MFILE);
	END;
       2:			  (* Команда в▒▓авки '+'  *)
	BEGIN
	 WRITELN(' МОДУЛЬ ',moda[jj].nam_mod,' В БИБЛИОТЕКЕ ЕСТЬ ');
{	  ERASE(WFILE);
	 CLOSE(WFILE);		  (* Уни╖▓ожи▓╝ п░омеж│▓о╖н╗й ┤айл *)
	 FLIG:=FALSE   }
	END;
       4:			   (* Команда │далени┐ - *)
	BEGIN
	 RDDL(RFILE);
	 ZMODA(RFILE,WFILE,NAME_MODT,FALSE,FALSE); (* П░оп│▒к мод│л┐ *)
	END;
       5:			   (* Команда │далени┐ и в╗бо░а *-  *)
	BEGIN
	 ASSIGN(MFILE,moda[jj].NAM_MOD);
	 REWRITE(MFILE,1);	 (* О▓к░╗▓╝ ┤айл мод│л┐ *)
	 IF IORESULT<>0 THEN BEGIN     (*  о╕ибка *)
	   WRITELN(' ERROR DISK ',moda[jj].NAM_MOD);
	   CLOSE (PFILE);
	   CLOSE (WFILE);
	   ERASE (WFILE);
	   CLOSE (RFILE);
	   WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
	   HALT(1)
	 END;
(*   мод│л┐ в библио▓еке *)
	 RDDL(RFILE);	     (* П░о╖и▓а▓╝ и п░еоб░азова▓╝ длин│ мод│л┐ *)
	 ZMODA(RFILE,MFILE,NAME_MODT,TRUE,FALSE);
	 CLOSE(MFILE)
	END;
       6:			    (* Команда замен╗ *)
	BEGIN
	 ASSIGN(MFILE,moda[jj].NAM_MOD);
	 RESET(MFILE,1);	       (* О▓к░╗▓╝ ┤айл мод│л┐ *)
	 IF IORESULT<>0 THEN BEGIN     (*  о╕ибка *)
	   WRITELN(' ERROR DISK ',moda[jj].NAM_MOD);
	   CLOSE (PFILE);
	   CLOSE (WFILE);
	   ERASE (WFILE);
	   CLOSE (RFILE);
	   WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
	   HALT(1)
	 END;
(*	 Пол│╖и▓╝ дилин│ ┤айла	*)
	 DLINA(MFILE);	(*DLF:=FILESIZE(MFILE);*)
	 BLOCKWRITE(WFILE,NAME_MODT,8);     (* Запи▒╝ имени мод│л┐ *)
	 BLOCKWRITE(WFILE,BUF,4);	    (* Запи▒╝ длин╗ мод│л┐ *)
	 ZMODA(MFILE,WFILE,NAME_MODT,TRUE,FLPR); (* Запи▒╝ мод│л┐ *)
	 CLOSE(MFILE);
(*
    Пе░епи▒а▓╝ мод│л╝ из библио▓еки в ┤айл
*)
	 RDDL(RFILE);	(* П░о╖и▓а▓╝ и п░еоб░азова▓╝ длин│ мод│л┐ *)
	 ZMODA(RFILE,MFILE,NAME_MODT,FALSE,FALSE); (* П░оп│▒к мод│л┐ *)
	END
       END   (* CASE *)
      END
(*
	    Не ▒овпада╛▓ имена мод│лей
*)
     ELSE  BEGIN
      RDDL(RFILE);		(*  Ч▓ение длин╗ *)
      IF FLIG THEN BEGIN	   (* Запи▒╝ веде▓▒┐? *)
	BLOCKWRITE(WFILE,NAME_MODT,8);	   (* Запи▒╝ имени мод│л┐ *)
	BLOCKWRITE(WFILE,BUF,4) 	  (* Запи▒╝ длин╗ мод│л┐ *)
      END;
      ZMODA(RFILE,WFILE,NAME_MODT,FLIG,FLPR)  (* Запи▒╝ мод│л┐ *)
    END     (* IF *)
  END;	    (* FOR В▒е мод│ли п░о▒мо▓░ели *)
  if j<>0 then
    for jj:=1 to j do
     begin

      IF moda[jj].KOM=2 THEN BEGIN		 (*  Команда в▒▓авки *)
	ASSIGN(MFILE,moda[jj].NAM_MOD);
	RESET(MFILE,1);       (* О▓к░╗▓╝ ┤айл мод│л┐ *)
	IF IORESULT<>0 THEN BEGIN     (*  о╕ибка *)
	  WRITELN(' ERROR DISK ',moda[jj].NAM_MOD);
	  CLOSE (PFILE);
	  CLOSE (WFILE);
	  ERASE (WFILE);
	  CLOSE (RFILE);
	  WRITELN(' LIB  АВАРИЙНОЕ ЗАВЕРШЕНИЕ');
	  HALT(1)
	END;
(*	 Пол│╖и▓╝ дилин│ ┤айла	*)
	DLINA(MFILE);
	BLOCKWRITE(WFILE,moda[jj].NAME_MOD,8);	     (* Запи▒╝ имени мод│л┐ *)
	BLOCKWRITE(WFILE,BUF,4);	   (* Запи▒╝ длин╗ мод│л┐ *)
	ZMODA(MFILE,WFILE,moda[jj].NAME_MOD,TRUE,FLPR); (* Запи▒╝ мод│л┐ *)
	CLOSE(MFILE)
      end;
     END;
  IF FLIG THEN BEGIN
(*    Пе░еименова▓╝ ┤айл RFILE B .BLI  *)
    CLOSE(RFILE);
    ERASE(RFILE);
    CLOSE(WFILE);
    RENAME(WFILE, LIB_NAM);

(*    Пе░еименова▓╝ ┤айл WFILE B .LIB  *)
    END
   ELSE
    CLOSE(RFILE);
    CLOSE(PFILE);
(*
     КОНЕЦ РАБОТЫ
*)
  WRITELN (' LIB  КОНЕЦ РАБОТЫ ');
 END.
