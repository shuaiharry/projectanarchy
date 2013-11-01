cl /Ox /Ob2 /Oi /Ot /Oy /GL /I ".." /GF /FD /EHsc /MD /GS- /Gy /fp:fast /c  /nologo /TC geom.c geom2.c global.c io.c mem.c merge.c poly.c poly2.c qhull.c qset.c stat.c unix.c user.c
lib /OUT:"..\lib\qhullx64.lib" /NOLOGO /LTCG *.obj 
