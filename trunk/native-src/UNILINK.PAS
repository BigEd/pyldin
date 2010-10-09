(******************************************************
*						      *
*	      РЕДАКТОР СВЯЗИ ОБ'ЕКТН╗Х МОДУЛЕЙ        *
*		       UniLINK			      *
*						      *
*	       18.07.1989 г.   17.10	V.1.1	      *
*	       (c) 1989 Leonid Curak		      *
*******************************************************)

(*
	     Опи▒ание ▓ипов
*)
   CONST  maxd=2000;		       { длина ма▒▒ива BUF }
	  maxt=15000;		       { длина на▒▒ива TABT }

   TYPE   name	 = array[1..16] of byte;

	  tmods  = record	       { ▒▓░│к▓│░а ▓абли╢╗ мод│лей TMOD }
	       hand  :byte;	       { номе░ ┤айла }
	       nmod  :longint;	       { пози╢и┐ в ┤айле }
	       sizec :integer;	       { ░азме░ кода }
	       ykpb  :integer;	       { │каза▓ел╝ ▓абли╢╗ PUBL }
	       ykext :integer;	       { │каза▓ел╝ ▓абли╢╗ EXT }
	       namem :array[1..8] of char; { им┐ мод│л┐ }
	   end;

	  tvmods = record	       { ▒▓░│к▓│░а ▓абли╢╗ и▒пол╝з│ем╗╡ мод│лей}
	       nom   :integer;	       { ▒▒╗лка на ▓абли╢│ TMOD }
	       bas   :integer;	       { база мод│л┐ }
	   end;

	  texs	 = record	       { ▒▓░│к▓│░а ▓абли╢╗ EXT }
	       namext:name;	       { им┐ ▓о╖ки ЕХТ }
	       adr   :integer;	       { зна╖ение ▓о╖ки }
	       tip   :byte;	       { ▓ип ▓о╖ки }
	   end;

	  tmodt  = array[1..256] of tmods;
	  tvmodt = array[1..256] of tvmods;
	  textt  = array[1..1000] of texs;

(*
	    Опи▒ание пе░еменн╗╡
*)
   VAR	  tabt			  : array[1..maxt] of byte; { б│┤е░ ▓о╖ек ЕХТ и PUBL }
	  buf			  : array[1..maxd] of byte; { б│┤е░ ╖▓ени┐/запи▒и }
	  rfile 		  : array[1..32] of file;   { │каза▓ели ┤айлов ╖▓ени┐ }
	  nam_mod		  : array[1..32] of string[40]; { имена ┤айлов ╖▓ени┐ }
	  mnm			  : name;
	  digits		  : array[1..16] of char;   { пе░екоди░овка }

	  tmod			  : tmodt;
	  tvmod 		  : tvmodt;
	  tex			  : textt;

	  adrzs,adrzm		  : byte;
	  flt,fls,fltp,flni,flv   : boolean;
	  wfile,pfile		  : file;
	  fprin 		  : text;
	  namfr,namfw,namfp,par   : string[40];
	  nfprin,namf		  : string;
	  pozf			  : longint;

	  kmodl,kmod,dlpb,dlext   : integer;
	  ktrel,sizect,sizev,kfm  : integer;
	  indm,indt,nmodt,kmoda   : integer;
	  i,i1,i2,it,ip,ir,znt	  : integer;
	  inex,kpb,kext,tbas,klrel: integer;

(*
	   ПРОЦЕДУР╗
*)

(*
      П░о╢ед│░а п░оп│▒ка ┤айла
*)
  procedure FCON( var kps:integer);
   var jj :integer;
   begin
    repeat
     if kps<maxd then
       jj:=kps
      else
       jj:=maxd;
     kps:=kps-jj;
     BLOCKREAD(rfile[i],buf,jj);
    until kps=0;
   end;

(*
	     П░о╢ед│░а об░або▓ки мод│л┐ на 1 п░о╡оде
*)
  procedure MODUL;

   var j  :integer;

   begin
    kpb:=buf[5]*256+buf[6];	     { коли╖. (.) PUBL }
    kext:=buf[7]*256+buf[8];	     { коли╖. (.) EXT }
    dlpb:=kpb*20;
    dlext:=kext*20;
    sizect:=buf[9]*256+buf[10];      { ░азме░ кода }
(*
	Заполнение ╜лемен▓а ▓абли╢╗ TMOD
*)
    tmod[indm].hand:=i; 	     { номе░ ┤айла }
    tmod[indm].nmod:=pozf;	     { пози╢и┐ мод│л┐ в ┤айле }
    tmod[indm].ykpb:=indt;
    tmod[indm].sizec:=sizect;	     { ░азме░ мод│л┐ }
    for j:=1 to 8 do		     { им┐ мод│л┐ }
      tmod[indm].namem[j]:=chr(mnm[j]);

(*
	Запи▒╝ ▓абли╢╗ PUBL
*)
    tabt[indt]:=buf[5];
    tabt[indt+1]:=buf[6];	      { коли╖е▒▓во (.) PUBL }
    indt:=indt+2;
    if dlpb<>0 then
      begin
       BLOCKREAD(rfile[i],buf,dlpb);   { ╖▓ение PUBL }
       for j:=1 to dlpb do
	begin
	 tabt[indt]:=buf[j];
	 indt:=indt+1;
	end;
      end
     else			       {не▓ PUBL поме▒▓им ▓о╖к│ зап│▒ка }
      if buf[15]*256+buf[16]<>0 then
       begin
	tabt[indt-1]:=1;
	for j:=1 to 16 do
	 begin
	  tabt[indt]:=mnm[j];
	  indt:=indt+1;
	 end;
	tabt[indt]:=buf[15];
	tabt[indt+1]:=buf[16];
	tabt[indt+2]:=0;
	tabt[indt+3]:=0;
	indt:=indt+4;
       end;

(*
	 Запи▒╝ ▓абли╢╗ EXT
*)
    tmod[indm].ykext:=indt;
    tabt[indt]:=HI(kext);
    tabt[indt+1]:=LO(kext);	  { коли╖е▒▓во (.) EXT }
    indt:=indt+2;
    if dlext<>0 then
      begin
       BLOCKREAD(rfile[i],buf,dlext);	{ ╖▓ение EXT }
       for j:=1 to dlext do
	begin
	 tabt[indt]:=buf[j];
	 indt:=indt+1;
	end;
      end;

(*
    П░оп│▒к до кон╢а мод│л┐
*)
    FCON(sizect);		     { п░оп│▒▓и▓╝ код }
    BLOCKREAD(rfile[i],buf,2);	     { коли╖е▒▓во (.) REL }
    j:=(buf[1]*256+buf[2])*5+1;      { длина REL + п░изнак кон╢а REL }
    FCON(j);			     { п░оп│▒▓и▓╝ REL }
    indm:=indm+1;		     { │вели╖и▓╝ коли╖е▒▓во мод│лей }
   end; 	 { PROC MODUL }


(*
     П░о╢ед│░а в╗делени┐ имени мод│л┐ из имени ┤айла
*)
   procedure MODNAM(nam:string; var mnmd:name);

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
	  mnmd[j]:=ORD(UPCASE(ch));
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
	 П░о╢ед│░а пе░вода в ╕е▒▓над╢а▓и░и╖ное ▒имвол╝ное п░ед▒▓авление
*)
  procedure SHEST( kod,nm:integer);
   var j,kk : integer;

   begin
    kk:=kod;
    for j:=0 to 3 do
     begin
      par[nm+4-j]:=digits[kk mod 16+1];
      kk:=kk div 16;
     end;
   end; 	  { PROC SHEST }

(*
	      П░о╢ед│░а запи▒и ▓абли╢╗ ЕХТ мод│л┐ в об╣│╛ ▓абли╢│ ТЕХ
*)
  procedure ZEXT;

   var kex,jp,je,jk,jt :integer;
       fle	       :boolean;

   begin
    jp:=tmod[indm].ykext;	   { │каза▓ел╝ ▓абли╢╗ мод│л┐ }
    kex:=tabt[jp]*256+tabt[jp+1];  { коли╖е▒▓во (.) EXT }
    jp:=jp+1;
    if kex<>0 then
     begin
      for it:=1 to kex do	   { пои▒к │же запи▒аной ▓о╖ки }
       begin
	fle:=false;
	if kext<>0 then
	 begin
	  jk:=1;

	  repeat
	    je:=1;

	    repeat
	      fle:=tabt[ip+je-1]=tex[jk].namext[je]; { ▒░авнение имен }
	      je:=je+1;
	    until (je>16) or not fle;

	    jk:=jk+1;
	  until (jk>kext) or fle;
	 end;
	if not fle then 	    { ▓акой ▓о╖ки не▓, поме▒▓им в ▓абли╢│ }
	 begin
	  kext:=kext+1; 	    { │вели╖и▓╝ коли╖е▒▓во ▓о╖ек }
	  for je:=1 to 16 do
	    tex[kext].namext[je]:=tabt[jp+je];
	  tex[kext].tip:=$ff;	    { п░изнак неоп░еделенной ▓о╖ки }
	 end;
	jp:=jp+20;		    { пе░е╡од к новой ▓о╖ке }
       end
     end;
   end; 	{ PROC	ZEXT }

(*
	  П░о╢ед│░а о▓к░╗▓и┐ ┤айла ╖▓ени┐
*)
  procedure OPENR(var nf:string; ii:integer);

   var jf : integer;
{$i-}
    begin
     ASSIGN(rfile[ii],nf);
     RESET(rfile[ii],1);
     if ioresult <> 0 then
      begin
       WRITELN(' LINK  ERROR FILE ',nf);
       if ii<>1 then
	 for jf:=1 to ii-1 do	     { зак░╗▓ие о▓к░╗▓╗╡ ┤айлоб ╖▓ени┐ }
	   CLOSE(rfile[jf]);
       HALT(2);
     end;
    end;	{ PROC	OPENR }
{$i+}

(*
       П░о╢ед│░а о╕ибки в мод│ле
*)
  procedure ERRMOD;
   var jf : integer;

   begin
    WRITELN('LINK  Файл ',namf,' не об║ек▓н╗й мод│л╝ ');
    for jf:=1 to i do	     { зак░╗▓ие о▓к░╗▓╗╡ ┤айлов ╖▓ени┐ }
      CLOSE(rfile[jf]);
    HALT(2);
   end; 	 { PROC ERRMOD }

(*
       П░о╢ед│░а о╕ибки в ┤айле
*)
  procedure ERRDISK(j : integer; nf : string);
   var jf : integer;

   begin
     WRITELN(' LINK  ERROR FILE ',nf);
     for jf:=1 to kfm do	{ зак░╗▓ие о▓к░╗▓╗╡ ┤айлов ╖▓ени┐ }
       CLOSE(rfile[jf]);
     if j>1 then
      begin
       ERASE(wfile);
       CLOSE(wfile);
      end;
      ERASE(fprin);
      CLOSE(fprin);
      HALT(2);
   end; 	   { PROC  ERRDISK }

(*
	 О╕ибка в па░аме▓░а╡
*)
  procedure ERRPAR;

   begin
    WRITELN(' LINK о╕ибка в па░аме▓░а╡ ');

    writeln('usage: UniLink MainModule [Modules] [/L<ListFile>] [/P]');
    WRITELN('       /P - па░аме▓░ в╗░авнивани┐ на г░ани╢│ $100 бай▓.');
    HALT(4);
   end; 	   { PROC ERRPAR }

(*
    Неоп░еделенн╗й иден▓и┤ика▓о░
*)
  procedure ERRNI;

   var stp : string[17];
       ji  : integer;

   begin
    stp:=' ';
    for ji:=1 to 16 do
      stp:=stp + char(tex[inex].namext[ji]);
    WRITELN(' Иден▓и┤ика▓о░ ',stp,' неоп░еделен.');
    flni:=true;
   end;

(*
	 НАЧАЛО РАБОТ

	  Блок ░азбо░а па░аме▓░ов
*)

begin
  writeln('UniLINK. Version 1.01. (c) 1989 "ТРИАДА".');
  digits:='0123456789ABCDEF';      { ини╢иализа╢и┐ пе░екоди░овки }
  nfprin:=' ';
  ip:=PARAMCOUNT;		   { коли╖е▒▓во па░аме▓░ов }
  if ip<1 then ERRPAR;
  kfm:=0;
  flv:=false;			   { не в╗░авнива▓╝ }
  for i:=1 to ip do		   { ╢икл ░азбо░а па░аме▓░ов }
   begin
    par:=PARAMSTR(i);		   { о╖е░едной па░аме▓░ }
    if par[1] = '/' then
(*	задание ┤айла пе╖а▓и	     *)
       if UPCASE(par[2])='P' then
	 flv:=true		   { │▒▓анави▓╝ в╗░авнивание }
	else
	 if UPCASE(par[2])='L' then
	   begin
	    nfprin:=par;
	    DELETE(nfprin,1,2);
	   end
	  else
	   ERRPAR
     else
      begin
       kfm:=kfm+1;		   { нов╗й мод│л╝ }
       it:=POS('.',par);           { ec▓╝ ░аc╕и░ение? }
       if it=0 then		   { не▓ }
	 par:=par+'.OBJ';
       nam_mod[kfm]:=par;
      end;
   end;
  if nfprin=' ' then
    nfprin:='CON';
  if kfm=0 then ERRPAR;

(*
      Пе░в╗й п░о╡од  LINK
*)
  indm:=1;				  { индек▒ ма▒▒ива ▓о╖ек TABT }
  indt:=1;				  { индек▒ ▓абли╢╗ мод│лей TMOD }

  for i:=1 to kfm do			  { ╢икл по ┤айлам }
   begin
    namf:=nam_mod[i];			  { им┐ о╖е░едного ┤айла }
    OPENR(namf,i);			  { о▓к░╗▓╝ ┤айл }
(*    nmodt:=0; 			{ ▓ек│╣ий номе░ мод│л┐ в библио▓еке}*)
    pozf:=0;				  { пози╢и┐ на╖ала ┤айла }
    BLOCKREAD(rfile[i],buf,32); 	  { ╖▓ение заголовка }

    MODNAM(namf,mnm);			  { в╗дели▓╝ им┐ мод│л┐ из имени ┤айла }
    if (buf[1]=$0b) and (buf[2]=$b0) then
      begin
(*
     Об░або▓ка библио▓еки
*)
       kmodl:=buf[5]*256+buf[6];	   { коли╖е▒▓во мод│лей в библио▓еке }
       for i1:=1 to kmodl do		   { ╢икл по мод│л┐м }
	begin
	 BLOCKREAD(rfile[i],mnm,12);	   { им┐ мод│л┐ и длина }
	 for i2:=9 to 16 do
	   mnm[i2]:=$20;		   { дополни▓╝ им┐ п░обелами }
	 pozf:=FILEPOS(rfile[i]);	   { пози╢и┐ мод│л┐ в ┤айле }
	 BLOCKREAD(rfile[i],buf,32);	   { заголовок мод│л┐ }
	 nmodt:=nmodt+1;		   { номе░ мод│л┐ }
	 MODUL; 			   { об░або▓а▓╝ мод│л╝ }
	end;
      end

     else				   { п░о▒▓ой мод│л╝ }
      if (buf[1]=$5a) and (buf[2]=$a5) then
	MODUL
       else
	ERRMOD;
   end; 	{ Koне╢ 1 п░о╡ода }
   kmoda:=indm; 			   { об╣ее коли╖е▒▓во мод│лей }

(*
	    Aлго░и▓м │▒▓ановки ▒в┐зок
*)
  kmod:=1;			  { ав▓ома▓и╖е▒кое вкл╛╖ение 1 мод│л┐ }
  flni:=false;			  { ┤лаг неоп░еделенного иден▓и┤ика▓о░а }
  tvmod[1].nom:=1;
  tvmod[1].bas:=0;		  { база мод│л┐ }
  kext:=0;			  { koли╖е▒▓во (.) EXT }
  tbas:=tmod[1].sizec;
  if flv then
    tbas:=((tbas + $ff) div $100) * $100;  { в╗░авнивание на г░ани╢│ ▒▓░ани╢╗ }
  indm:=1;
  ZEXT; 			  { поме▒▓и▓╝ блок EXT в ▓абли╢│ TEX }

(*
   Оп░еделение EXT
*)
  if kext<>0 then	      { IF0 }
   begin
    inex:=1;			  { индек▒ EXT }

    repeat		     {REP 1 }
      indm:=0;			  { индек▒ мод│л┐ в TMOD }
      flt:=false;		  { ┤лаг найденой ▓о╖ки }

      repeat		      {REP 2 }
	indm:=indm+1;
	ip:=tmod[indm].ykpb;	  { │каза▓ел╝ PUBL }
	kpb:=tabt[ip]*256+tabt[ip+1];
	ip:=ip+1;
	if kpb <> 0 then
	 begin
	  i1:=1;

	  repeat		{ REP 3 }
	    i2:=1;

	    repeat		   { REP 4 }
	      flt:=tex[inex].namext[i2] = tabt[ip+i2];	 { ▒░авнение имен }
	      i2:=i2+1;
	    until (i2>16) or not flt;	     {END REP 4 }

	    i1:=i1+1;
	    ip:=ip+20;		     { ад░е▒ ▒лед│╛╣его EXT }
	  until (i1>kpb) or flt;  { END REP 3 }
	 end;
      until (indm>kmoda) or flt; {END REP 2 }

      if not flt then ERRNI	    { неоп░еделен EXT }
       else
	begin	      {  IF2}
(*
	 На╕ли ▒оо▓ве▓▒▓вие EXT - PUBL
	  INDM - номе░ мод│л┐ б TMOD
	 п░ове░┐ем по┐вл┐л▒┐ ли мод│л╝ ░анее
*)
	 ip:=ip-3;		    { индек▒ (.) PUBL ее ад░е▒а и ▓ипа }
	 i1:=0;
	 repeat
	   i1:=i1+1;
	   flt:=tvmod[i1].nom=indm;   { е▒▓╝ ли мод│л╝ в TVMOD }
	 until (i1=kmod) or flt;

	 if not flt then
	  begin
(*
	 Нов╗й мод│л╝ поме╣аем его в TMOD
	  и под▒оедин┐ем его ЕХТ
*)
	   kmod:=kmod+1;
	   tvmod[kmod].nom:=indm;
	   tvmod[kmod].bas:=tbas;	{ база мод│л┐ }
	   tbas:=tbas+tmod[indm].sizec;
	   if flv then			 { в╗░овн┐▓╝? }
	     tbas:=((tbas + $ff) div $100) * $100;  { в╗░авнивание на г░ани╢│ ▒▓░ани╢╗ }
	   ZEXT;			{ под▒оедини▓╝ EXT }
	   i1:=kmod;
	  end;

	 if tabt[ip+2] <> 0 then	{ п░ове░ка ▓ипа }
	   tex[inex].adr:=tabt[ip]*256+tabt[ip+1]   { CONST }
	  else
	   tex[inex].adr:=tabt[ip]*256+tabt[ip+1]+tvmod[i1].bas;  { ADRES }

	 tex[inex].tip:=tabt[ip+2];	 { ▓ип }
	end; {	 IF2 }
      inex:=inex+1;
    until inex > kext;	  { END REP 1 }
   end; 	    { END IF 0 }

(*
      П░ове░ка ┤лага неоп░еделенно▒▓и
*)
  if flni then			   { б╗ли неоп░еделенн╗е иден▓и┤ика▓о░╗ }
    begin			   { заве░╕аем ░пабо▓│ }
     for i:=1 to kfm do
       CLOSE(rfile[i]);
     WRITELN('  LINK  Ава░ийное заве░╕ение ');
     HALT(1);
    end;
(*
      В▓о░ой п░о╡од  LINK
*)
  namfw:=nam_mod[1];	       { пол│╖аем им┐ ░ез. ┤айла}
  i:=POS('.',namfw);           { номе░ ▓о╖ки }
  if i<>0 then DELETE(namfw,i,4); { │дали▓╝ па▒╕е░ение }
  namfp:=namfw + '.$pg';         { им┐ п░ом.┤айла }
  namfw:=namfw + '.pgm';
(*
	О▓к░╗▓ие ░ез│л╝▓а▓а и п░ом.┤айла.
*)
  ASSIGN(fprin,nfprin);
  REWRITE(fprin);
  if ioresult<>0 then ERRDISK(0,nfprin);
  {$i-}
  ASSIGN(wfile,namfw);
  REWRITE(wfile,1);
  if ioresult<>0 then ERRDISK(1,namfw);

  ASSIGN(pfile,namfp);
  REWRITE(pfile,1);
  if ioresult<>0 then ERRDISK(2,namfp);

  WRITELN(fprin,'           Заг░│зо╖н╗й мод│л╝ ',namfw);
  WRITELN;
  {$i+}
  for i:=1 to 16 do		    { ╢икл обн│лени┐ BUF }
    buf[i]:=0;
  BLOCKWRITE(wfile,buf,16);	    { заби▓╝ заголовок }
(*
       Цикл по ▓абли╢е вкл. мод│лей TVMOD
*)
  klrel:=0;			    { коли╖е╢▓во ▓о╖ек REL }

  for i:=1 to kmod do		    { FOR1 оcновной ╢икл }
   begin
    i1:=tvmod[i].nom;		     { номе░ в TMOD }
    i2:=tmod[i1].hand;		     { номе░ ┤айла }

    if FILEPOS(rfile[i2])<>tmod[i1].nmod then
      SEEK(rfile[i2],tmod[i1].nmod);  { │c▓ановка ┤айла на на╖ало мод│л┐ }

    BLOCKREAD(rfile[i2],buf,32);     { заголовок мод│л┐ }

    if i=1 then
     begin			   { запомни▓╝ ад░е▒ зап│▒ка п░ог░амм╗ }
      adrzs:=buf[15];
      adrzm:=buf[16];
     end;

    kpb:=buf[5]*256+buf[6];	     { коли╖. (.) PUBL }
    kext:=buf[7]*256+buf[8];	     { коли╖. (.) EXT }
    dlpb:=kpb*20;
    dlext:=kext*20;
    sizect:=buf[9]*256+buf[10];      { ░азме░ кода }

    if dlpb<>0 then
      BLOCKREAD(rfile[i2],buf,dlpb);	    { п░оп│▒▓и▓╝ PUBL }

    if dlext<>0 then
      BLOCKREAD(rfile[i2],buf,dlext);
    BLOCKREAD(rfile[i2],tabt,sizect);  { п░о╖и▓али код }

    sizev:=sizect;
    if flv then
      sizev:=((sizect+$ff) div $100)* $100; { в╗░овнен╗й код }
      if sizev<>sizect then
	for it:=sizect+1 to sizev do
	  tabt[it]:=0;			  { дополни▓╝ код н│л┐ми }

(*
       В╗вод на пе╖а▓╝ имени мод│л┐ и его баз╗
*)
    WRITELN(fprin,'   ',tmod[i1].namem);
    par:='            ';
    SHEST(tvmod[i].bas,1);
    ir:=tvmod[i].bas+sizect-1;
    SHEST(ir,7);
    WRITELN(fprin,'          ',par);

(*
     Цикл по ▓о╖кам REL ▒ и▒п░авлением кода
*)
    BLOCKREAD(rfile[i2],mnm,2); 	{ ╖▓ение коли╖е▒▓ва ▓о╖ек }
    ktrel:=mnm[1]*256+mnm[2];

    if ktrel<>0 then
      for ir:=1 to ktrel do
       begin
	fltp:=true;			  { ┤лаг ▓ипа (.) - REL }
	BLOCKREAD(rfile[i2],mnm,5);	  { ╜лемен▓ REL }
	znt:=mnm[2]*256+mnm[3]; 	  { зна╖ение }

	if mnm[1] < $80 then		  { п░о▒▓а┐ ▓о╖ка }
	  znt:=znt+tvmod[i1].bas	  { бази░ование ▓о╖ки }

	 else				  { вне╕н┐┐ ▓о╖ка }
	  begin
	   znt:=znt*20; 	      { ▒ме╣ение б ▓абли╢е EXT ▒ н│л┐ }
	   inex:=0;			  { индек▒ TEX }
(*
	 пои▒к вне╕ней ▓о╖ки в TEX
*)
	   repeat
	     inex:=inex+1;
	     it:=1;
	     repeat
	       flt := tex[inex].namext[it] = buf[znt+it]; { п░изнак ▒░авнени┐ }
	       it:=it+1;
	     until (it > 16) or not flt;     { ╢икл до кон╢а или не ░авно }
	   until (inex = kext) or flt;	{ ╢икл до кон╢а или  ░авно }

	   if not flt then		   { ▓о╖ка не найдена }
	     begin
	      fltp:=false;		    { ▓ип CONST }
	      znt:=0;			    { зна╖ение неоп░еделено }
	     end
	    else			    { ▓о╖ка  найдена }
	     begin
	      fltp:=tex[inex].tip=0;
	      znt:=tex[inex].adr;
	     end;
	  end;	    {IF}

(*
	 И▒п░авление кода
*)
	  it:=mnm[4]*256+mnm[5]+1;	    { ▒ме╣ение в коде дл┐ и▒п░авлени┐ }
	  fls:=mnm[1] and 2 = 2;		   { ┤лаг ▒▓а░╕его бай▓а }
	  if ODD(mnm[1]) then		     { ┤лаг млад╕его бай▓а ╖е▓но▒▓╝ }
	    tabt[it+1]:=LO(znt);	{ и▒п░ави▓╝ млад╕ий бай▓ }
	  if fls then
	   begin
	    tabt[it]:=HI(znt);	{ и▒п░ави▓╝ ▒▓а░╕ий бай▓ }

	    if fltp then		    { ▓о╖ка REL }
	     begin
	      it:=it+tvmod[i].bas-1;	    { ▒ме╣ение в заг░│зо╖ном мод│ле }
	      mnm[1]:=HI(it);
	      mnm[2]:=LO(it);
	      klrel:=klrel+1;
	      BLOCKWRITE(wfile,mnm,2);	   { в╗ве▒▓и о╖е░едн│╛ ▓о╖к│ }
	     end;
	   end;
       end;	 {FOR по ▓о╖кам REL }

    BLOCKWRITE(pfile,tabt,sizev);	    { в╗ве▒▓и и▒п░авленн╗й код }
   end;       {FOR по мод│л┐м }

(*
      Пе░епи▒а▓╝ код из п░омеж│▓о╖ного ┤айла в ░ез│л╝▓и░│╛╣ий
*)
  pozf:=0;
  SEEK(pfile,pozf);			    { │▒▓анови▓╝ ┤айл в на╖ало }
  dlpb:=tbas;				    { длина кода }
  pozf:=FILEPOS(wfile); 		    { запомни▓╝ пози╢и╛ ░ез.┤айла }
  repeat
    if dlpb < maxd then it:=dlpb else it:=maxd;
    dlpb:=dlpb-it;
    BLOCKREAD(pfile,tabt,it);
    BLOCKWRITE(wfile,tabt,it);
  until dlpb = 0;
(*
	Фо░ми░ование и запи▒╝ заголовка
*)
  buf[1]:=$a5;
  buf[2]:=$5a;
  buf[3]:=HI(klrel);
  buf[4]:=LO(klrel);
  buf[5]:=(pozf div 256) mod 256;
  buf[6]:=pozf mod 256;
  buf[7]:=HI(tbas);
  buf[8]:=LO(tbas);
  buf[9]:=adrzs;
  buf[10]:=adrzm;

  pozf:=0;
  SEEK(wfile,pozf);			    { │▒▓анови▓╝ ┤айл в на╖ало }
  BLOCKWRITE(wfile,buf,10);
(*
       Зак░╗▓ие в▒е╡ ┤айлов и заве░╕ение ░або▓╗
*)
  for i:=1 to kfm do
    CLOSE(rfile[i]);
  CLOSE(wfile);
  ERASE(pfile);
  CLOSE(pfile);
  CLOSE(fprin);
  WRITELN('   LINK  Kоне╢ ░або▓╗ ');
  HALT(0);
 end.
