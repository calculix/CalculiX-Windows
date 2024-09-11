/* --------------------------------------------------------------------  */
/*                          CALCULIX                                     */
/*                   - GRAPHICAL INTERFACE -                             */
/*                                                                       */
/*     A 3-dimensional pre- and post-processor for finite elements       */
/*              Copyright (C) 1996 Klaus Wittig                          */
/*                                                                       */
/*     This program is free software; you can redistribute it and/or     */
/*     modify it under the terms of the GNU General Public License as    */
/*     published by the Free Software Foundation; version 2 of           */
/*     the License.                                                      */
/*                                                                       */
/*     This program is distributed in the hope that it will be useful,   */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of    */ 
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the      */
/*     GNU General Public License for more details.                      */
/*                                                                       */
/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software       */
/*     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.         */
/* --------------------------------------------------------------------  */



#include <cgx.h>


#define     TEST            0   /* debugging */
#define     TEST1           0   /* debugging, substitute nurbs is kept */

extern Display       *dpy;
extern int           dpycells;
extern Colormap      cmap;
extern XColor        *xcolor;
extern unsigned long *pixels_return;
extern unsigned int  npixels;

extern int   w0, w1;                                      /* Fenster identifier  */
extern int   activWindow;                                 /* das aktuelle Fenster */
extern char  inpformat;  
extern int   width_w1, height_w1;
extern double   aspectRatio_w1;                            /* width_w1/height_w1 */
extern double trackbsize;                                  /* TRACKBALLSIZE */
extern double curquat[4];                                  /* Matrix aus Trackball */
extern double lastquat[4];                                 /* letzte Matrix aus Trackball*/

extern int   col_maxc,col_minc;
extern Entitycol *entitycol;
extern double     gtol;
extern int   steps;                                  /* Schrittweite der Farbscala */
extern int   offset, maxIndex;                       /* offset+steps-1 = maxIndex */
extern double dtx, dty, drx, dry, drz, ds;            /* Verschiebungen */
extern double centerPnt[3];                           /* Rotationszentrum */
extern double centerNode;               
extern double dx ,dy;                                 /* Mauskoordinaten im bereich +-1*/
extern int   xpixels ,ypixels;                       /* Mauskoordinaten in pixel, links unten 0,0 */
extern GLint   gl_max_eval_order;                         /* max order of NURBS */
extern GLdouble R[4][4];                                   /* Rotationsmatrix */
extern GLdouble Rmem[4][4];
extern double v[4];                                        /* drehkorrekturen fuer centerPkt */
extern char  zoomFlag;                     /* (1) zoomModus */
extern char  centerFlag;                   /* (1) search centerPnt */
extern char  printFlag;                    /* printf 1:on 0:off */
extern char  flipColorFlag;                 /* 0: high values use red, low use blue in scale; 1: flipped */
extern char  graphFlag;
extern int   cur_entity;                                       /* aktive entity (component), entity in menu= cur_entity+1*/
extern int       cur_lc;                                          /* aktive Dataset */
extern char  datin[MAX_LINE_LENGTH];                          /* Input-data-file */
extern int   ddiv;

extern char  mode[2];                        /* pickmode */
extern char  pickfunc[MAX_LINE_LENGTH];                     /* pickfunc either "qenq" "qadd" "qrem"   */

extern int  pick_zmin;                         /* kleinster z-wert der gepickten items */
extern char pickname[MAX_LINE_LENGTH];                     /* name of the picked item   */
extern char  setname[MAX_LINE_LENGTH];                     /* setname-buffer for pick() */

extern int     num_etype[13];

extern Scale     scale[1];
extern Elements  *e_enqire;     /* elem-array by elem-number instead of elem[index]... */
extern Summen    anz[1];
extern Edges     *edge;
extern Nodes     *node;
extern Datasets *lcase;
extern Faces     *face;

extern Alias     *alias;
extern Sets      *set;
extern Psets     *pset;
extern Values    *value;
extern Points    *point;
extern Lines     *line;
extern Lcmb      *lcmb;
extern Gsur      *surf;
extern Gbod      *body;
extern Nurbl     *nurbl;
extern Nurbs     *nurbs;
extern Shapes    *shape;
extern SumGeo    anzGeo[1];
extern SumAsci   sumAsci[1];


/* the copied node-sets which have to be filled with values from new loaded Datasets */
extern CopiedNodeSets copiedNodeSets[1];

/* additional entities */
extern char **valuestack;
extern int valuestack_ptr, valuestackFlag;
extern int       offset;
extern SpecialSet specialset[1];
extern int set_bsur, set_glur;

extern char **parameter;

extern sem_t   sem_g;
extern sem_t   sem_rep;


/*---------------------------------------------------------*/
/* Liste aller unterstuetzten Funktionen                   */
/*---------------------------------------------------------*/
void help( void )
{
  // remember for doc:
  // swep, move etc 'rad' [<p1> <p2> <value> <div>] missing in latex doc
  //
  printf ("\n---------------------------------------------------------\n");
  printf ("\n             Quick help for the experienced user \n");
  printf ("\n---------------------------------------------------------\n");
  printf ("\n\nEnvironment variables evaluated at startup (default values defined in cgx.h): \n");
  printf (" CGXDOCS:  Overwrites the cgx helpfile\n");
  printf (" CCXDOCS:  Overwrites the ccx helpfile\n");
  printf (" BROWSER:  Overwrites the helpfile browser\n");
  printf (" CGXTERMVIEWER: Overwrites the postscript viewer\n");
  printf (" CGXINITFILE:  Overwrites the initfile, default is .cgx \n");
  printf (" HOME: Overwrites the homepath were the initfile is located (Linux)\n");
  printf (" HOMEPATH:  see HOMEDRIVE \n");
  printf (" HOMEDRIVE: HOMEPATH plus HOMEDRIVE overwrite the homepath were the initfile is located (ms windows)\n");
  printf ("\n\nSpecial Keys: \n");
  printf (" ARROW_UP:   previous command \n");
  printf (" ARROW_DOWN: next command \n");
  printf (" PAGE_UP:    entities of previous set (if the last command was plot or plus) or the previous Loadcase\n");
  printf (" PAGE_DOWN:  entities of next set (if the last command was plot or plus) or the next Loadcase\n\n");
  printf ("\nMasking of entity names (alias) with a leading character: \n");
  printf (" '!':    uses the successive name as an alias and generates a new unused entity name\n");
  printf (" '\%%':    uses the successive name as an alias and replaces it by the referenced entity name\n");
  printf (" '?':    uses the successive name as an entity name and replaces it by the referenced alias (only 'prnt')\n");
  printf ("\nKnown commands and syntax: \n");
  printf (" '..':   Keyword (either uppercase or lowercase)\n");
  printf (" <..>:   Parameter (case-sensitive)\n");
  printf (" [..]:   combination of parameters or optional parameter \n");
  printf (" (..):   Remark \n");
  printf (" |   :   OR \n");
  printf (" &   :   AND \n");
  printf (" -   :   from-to \n");
  printf (" *chars*: wildcards are permitted\n");
  printf (" RETURN: return-key to press \n\n");
  printf ("   'allow_sys' (only in .cgx)\n");
  printf ("   'anim' 'start'|'tune' <value>|'steps' <value>|'time' <value>|'real' ['on'|'off']|'model' ['on'|'off']|'elem' ['on'|'off']\n");
  printf ("   'area' <set> \n");
  printf ("   'asgn' 'n'|'e'|'p'|'l'|'c'|'s'|'b'|'S'|'L'|'se'|'sh'|'alpha'|'beta'|'nadapt' <value>|'netgen'|'tetgen'|'thrds' <value>|['rbe' <value>|'mpc']|'max'|'maxr'|'maxc' [<col>]|'minc' [<col>]|['mem' 'free'|'keep']|'usr' <text>]|[graph on|off|<nr>|[bg on|off]|'viewer' <[path]file>|'viewformat' 'ps'|'png'|'browser' <[path]file>|'cgxhelpfile' <[path]file>|'ccxhelpfile' <[path]file>|'aflibhelpfile' <[path]file>\n");
  printf ("   'aver' <set> \n");
  printf ("   'bia'  [<set> [<bias> [<factor>]]|['mult'|'div' <factor>]] | [<line> <bias>]\n");
  printf ("   'body' <name|!> [<set>]|[<surf1> <surf2>]|[<surf1> <surf2> <surf3> <surf4> <surf5> [<surf6>] [<surf7>]]\n");
  printf ("   'break'\n");
  printf ("   'call' <parameters> \n");
  printf ("   'cmap' 'classic'|'grey'|'inferno'|'jet'|'coolwarm'|'turbo'|'viridis' (two significant char)\n");
  printf ("   'cntr' <pnt|nod|set>|[x y z] \n");
  printf ("   'col' <name> <red> <green> <blue> [<maxvalue>]\n");
  printf ("   'comp' <set|*chars*> 'c'|'d'|'e'|'u'|'f'\n");
  printf ("   'cont' (continue after stop)\n");
  printf ("   'copy' <set> <new_set> ['scal' <fx> <fy> <fz> [<pnt>]]|['tra' <x> <y> <z>]|['rad' [<p1> <p2> <value>]|['x'|'y'|'z' <value>]|[<p1> 'x'|'y'|'z' <value>]|['p'<PNT> <value>]]|['rot' [<p1> <p2> <value>]|['x'|'y'|'z' <value>]|[<p1> 'x'|'y'|'z' <value>]]|['mir' [<p1> <p2>]|['x'|'y'|'z']|[<p1> 'x'|'y'|'z']]|['nor' <value> [<div>]] [append]\n");
  printf ("   'corrad' <set>\n");
  printf ("   'csysa' <sysNr> <set>\n");
  printf ("   'cut' [<set>|<nod>] | [<pnt|nod> <pnt|nod> <pnt|nod>]\n");
  printf ("   'del'  ['p'|'l'|'l0'|'s'|'b'|'t'|'S'|'L'|'se'|'sh' <entity>]|['se0']|['mesh']|['pic']\n");
  printf ("   'dist' <set>|[<set> <set>|<shpe> []]|['tra' <x> <y> <z> <offset> [<tol>]]|['rad' [<p1> <p2> <offset>]|['x'|'y'|'z' <offset>] [<tol>]]|['rot' [<p1> <p2> <offset>]|['x'|'y'|'z' <offset> [<tol>]]]|['nor' <offset> [<tol>]]\n"); 
  printf ("   'div'  []|[<default>]|[<line> <div>]|[<set> ['auto' <node-dist> <angle> <elem-ratio>]|['mult'|'div' <factor-div> [<factor-bias>]]]\n");
  printf ("   'ds' [[<1.Dataset-Nr>|'l'|<negative-ds> [<2.Dataset-Nr>|'l'|<negative-ds> [<3.Dataset-Nr>|'l'|<negative-ds>]]] ['a[h]' [<entity-nr>]]|['e[h]' <entity-nr> (up to 4 for vector-plots)]|['o' <offset> [<entity-nr>]]|['p' <power>] [<entity-nr>]]|['s' <factor> [<entity-nr>]]]|[['+'|'-'|'*'|'/'] <Dataset-Nr> ['c']]|['g' <name> [[<ncomps>|<0>] <value> <text> <type> <step> <analysisName>]]|['e' <name> <comp> <type> <row> <column>]|['f']|['r' <key> [<parm1>] [..<parm5>]\n");
  printf ("   'elem' <nr|!|'fix'> [set]|[<firstNode>-<lastNode> 'be2'|'be3'|'tr3'|'tr6'|'qu4'|'qu8'|'he8'|'he20']\n");
  printf ("   'else'\n");
  printf ("   'else if' <value> 'eq'|'ne'|'=='|'!='|'<'|'>' <value>\n");
  printf ("   'elty' []|[<set>]|[<set> 'loc'|'ulock'|'be2'|'be2r'|'be2f'|'be2d'|'be3'|'be3r'|'be3f'|'tr3'|'tr3u'|'tr3e'|'tr3s'|'tr3c'|'tr6'|'tr6u'|'tr6e'|'tr6s'|'tr6c'|'qu4'|'qu4e'|'qu4s'|'qu4c'|'qu4r'|'qu4er'|'qu4sr'|'qu4cr'|'qu8'|'qu8e'|'qu8s'|'qu8c'|'qu8r'|'qu8er'|'qu8sr'|'qu8cr'|'he8'|'he8f'|'he8i'|'he8r'|'he20'|'he20r'|'te4'|'te4f'|'te10'|'te10m'|'te10t'|'pe6'|'pe6f'|'pe6i'|'pe15'|'pe15r' [<parameter>]]\n");
  printf ("   'endif'\n");
  printf ("   'endwhile'\n");
  printf ("   'enq' <set> <set> ['set' name]|'rec' <value>|'_' <value>|'_' <value>|'_']|['cx'|'cy'|'cz' <value>|'_'  <value>|'_' <value>|'_'] <tolvalue> 'i'|'a'|'h'|'l' [value]\n");
  printf ("   'eprop' <set>\n"); 
  printf ("   'eqal' 'jbir'|'aspr'|'mca' <value>\n");
  printf ("   'exit' \n");
  printf ("   'fil' <line> <line> <radius>\n");
  printf ("   'flip' [<set> <e>|<b>|<s>]|[<s> ['auto']]\n");
  printf ("   'flpc' \n");
  printf ("   'font' 'd'|'l' <value(1-6)> \n");
  printf ("   'frame' [<setname>]\n");
  printf ("   'gbod' <name|!> 'NORM'|'ADD' ['+|-' <surf>] ..  \n");
  printf ("   'gonly' 'on'|'off' \n");
  printf ("   'graph' [<amplitude|*chars*> 'amp' [<l>]]|[<material|*chars*> 'mat' [<l>]]|[<set>|'-p' ['length' ['+'|'-'|'+c'|'-c']]|['step'|'nr'|'freq'|'time'|'descr'|<parameter> [<dataset> <entity|parameter> [<first-Dataset-Nr> [<last-Dataset-Nr>]] ] ] ] \n");
  printf ("   'grpa' <grpNr> <set> ['dyn']\n");
  printf ("   'grps' \n");
  printf ("   'gsur' <name|!> ['+|-' 'BLEND|<nurbs>']|['ADD'] ['+|-' <line|lcmb>] .. \n");  
  printf ("   'gtol' []|<auto>|<geometric-tol> \n");
  printf ("   'help' \n");
  printf ("   'hcpy' [['ps'|'xwd'|'gif'|'png'] ['name']]|[make [ls]]|[clean] (def:xwd)\n");
  printf ("   'if' <value> 'eq'|'ne'|'=='|'!='|'<'|'>' <value>\n");
  printf ("   'int' <line> <line>\n");
  printf ("   'init' <internal parameters>\n");
  printf ("   'lcmb' <name|!> ['+|-' <line> '+|-' <line> ..(up to 14 lines)]|['ADD' '+|-' <line> '+|-' <line>..(up to 14 lines)] \n");
  printf ("   'length' <set>\n");
  printf ("   'line' <name|!> <p1> <p2> <cp|seq> <div> [<bias>]\n");
  printf ("   'lnor' <name|!> [<set>|[<p1> <p2> <p3>]] <length>\n");
  printf ("   'map' <slave-set> <master-set> ['volu' 'ds'[<nr>] ]|['surf' 'ds'[<nr>] ]|[ 'x'|'y'|'z'|'rx'|'ry'|'rz' 'ds'[<nr>] ] \n");
  printf ("   'mata' <matNr> <set>\n");
  printf ("   'mats' \n");
  printf ("   'max' <value> ['e'|'f'|'i'|'l'] ['l'|'u']\n");
  printf ("   'maxr' <value> ['e'|'f'|'i'|'l'] ['l'|'u']\n");
  printf ("   'maxc' <value> ['e'|'f'|'i'|'l'] ['l'|'u']\n");
  printf ("   'menu' <name> <command>\n");
  printf ("   'merg' 'n'|'e'|'p'|'l'|'c'|'s' <set> <gtol> 'nolock' \n");
  printf ("   'mesh' <set> ['fast'] ['tet' <size>|'block'|'lonly'|'nolength'|'noangle'|'length'|'angle']\n");
  printf ("   'mids <set> ['force'|'lin'|'gen'|'rem']\n");
  printf ("   'min' <value> ['e'|'f'|'i'|'l'] ['l'|'u']\n");
  printf ("   'minr' <value> ['e'|'f'|'i'|'l'] ['l'|'u']\n");
  printf ("   'minc' <value> ['e'|'f'|'i'|'l'] ['l'|'u']\n");
  printf ("   'minus' 'n'|'e'|'p'|'l'|'s'|'b'|'S'|'L'|'sh' <set> \n");
  printf ("   'move' <set> ['equ' <trgt-set> [<tol>]]|['scal' <fx> [<fy> <fz>] [<pnt>]]|['tra' <dx> <dy> <dz>]|['rot' [<p1> <p2>|['x'|'y'|'z'] <alfa>]|['x'|'y'|'z' [<alfa>|[<alfa1> <ax1> <alfa2> <ax2>]]]]||['rad' [<p1> <p2>|['x'|'y'|'z'] <alfa>]|['x'|'y'|'z'|'p'<pnt> [<dr>|<dr1> <ax1> <dr2> <ax2>]]]['mir' <P1> <P2>|['x'|'y'|'z']] \n");
  printf ("   'movi' [loops <nr>]|[delay <sec>]|[start]|[stop]|[frames ['auto']|[<nr> [<epilogFile>]]]|[make <nr> <nr> <prolog.gif>]|[clean] \n");
  printf ("   'mm' <value> ['f'|'i'|'e'] ['l'|'u']\n");
  printf ("   'msg' 'on|off' \n");
  printf ("   'mshp' <name> 's'|'b' <element-type-nr> <element-attr-nr> <lockFlag> <density>|<size> \n");
  printf ("   'neigh' <set> <tol> ['abq'|'ans'|'nas'] ['con'|'nsc' ['tie']|[[<stiffness>] [<mue>]]]|['equ' [<dofs('t'|'1-6')> 'c'|'u']|['tie' ['yes']]\n");
  printf ("   'node' <nr|!> [<x> <y> <z> ['0'|'1']]|['v' <value> [<value> .. ]]|['vs' <value>] \n");
  printf ("   'norm' <set> \n");
  printf ("   'nurl'  <name|!> ['DEFINE' ['COMPACT'] <pstart> <pend> <deg> <npnt> <nknt> <div>]|['CONTROL' <index> [<pnt>|<x y z>] <weight>]|['KNOT' <index> <value>]|['END']\n");
  printf ("   'nurs' [<name|!> ['DEFINE' ['COMPACT'] <u.deg> <v.deg> <u.npnt> <v.npnt> <u.nknt> <v.nknt>]|['CONTROL' <u.indx> <v.indx> [<pnt>|<x y z>] <weight>]|['KNOT' <u.indx> <v.indx> <value>]|['END']] | [ <!> <setname(containing surfaces)>]\n");
  printf ("   'ori' <set>\n");
  printf ("   'plot' ['n'|'e'|'f'|'p'|'l'|'s'|'b'|'S'|'L'|'sh']&['a'|'b'|'c'|'d'|'n'|'p'|'q'|'t'|'v'] [<set>|<*chars*>]] ['w'|'k'|'n'|'r'|'g'|'b'|'y'|'m'|'t'|'c'|'o'] [<width>]\n");
  printf ("   'plus' ['n'|'e'|'f'|'p'|'l'|'s'|'b'|'S'|'L'|'sh']&['a'|'b'|'c'|'d'|'n'|'p'|'q'|'t'|'v'] [<set>|<*chars*>]] ['w'|'k'|'n'|'r'|'g'|'b'|'y'|'m'|'t'|'c'|'o'] [<width>]\n");
  printf ("   'pnt' <name|!> [<x> <y> <z>]|[<L1> <ratio> <times>]|[<P1> <P2> <ratio> <times>]|[<setname(containing nodes)>]\n");
  printf ("   'prnt' ['ve']|['in']|['st' ['size']]|['usr']|['par' [<parameter>]|['amp' [<amplitude>|<*chars*>]|['mat' [<material>|<*chars*>]|['se'|'sq'|'eq' [<set>|<*chars*>]|['n'|'e' [<set>|<*chars*> ['range']]|['n'|'e'|'f'|'p'|'l'|'s'|'b'|'v'|'S'|'L' <entity>]\n");
  printf ("   'proj' <set> <set>|<shpe> ['tra' <x> <y> <z> <offset> [<tol>]]|['rad' [<p1> <p2> <offset>]|['x'|'y'|'z' <offset>] [<tol>]]|['rot' [<p1> <p2> <offset>]|['x'|'y'|'z' <offset> [<tol>]]]|['nor' <offset> [<tol>]]\n"); 
  printf ("   'qadd' <set> ['t'<value>] RETURN 'w'|'a'|'i'|'r'|'n'|'e'|'f'|'p'|'l'|'s'|'b'|'L'|'S'|'h'|'m'|'t'|'q'|'u'\n");
  printf ("   'qali' RETURN 'w'|'p'|'n'|'q' \n");
  printf ("   'qbia' RETURN 'w'|'a'|'i'|'1'-'9'|'c'|' '\n");
  printf ("   'qbod' <name>(optional) RETURN 'w'|'b'|'a'|'i'|'r'|'s'|'g'|'q'|'u'\n");
  printf ("   'qcnt' RETURN 'w'|'n'|'p' \n");
  printf ("   'qcut' RETURN 'w'|'n'|'p'|'q'|'v'|'x'|'y'|'z'|'s'\n");
  printf ("   'qdel' RETURN 'w'|'p'|'l'|'s'|'b'|'L'|'S'|'h'|'t'|'q'\n");
  printf ("   'qdis' RETURN 'w'|'c'|'f'|'g'|'n'|'m'|'p'|'q'|'s'\n");
  printf ("   'qdiv' RETURN 'w'|'a'|'i'|'1'-'9'|' '\n");
  printf ("   'qenq' RETURN 'w'|'a'|'i'|'r'|'n'|'e'|'f'|'p'|'l'|'s'|'b'|'L'|'S'|'h'|'m'|'t'|'q'\n");
  printf ("   'qfil' <radius> RETURN 'w'|'l'|'q' \n");
  printf ("   'qflp' RETURN 'w'|'e'|'s'|'a'|'i'|'q' \n");
  printf ("   'qint' RETURN 'w'|'l'|'q' \n");
  printf ("   'qlin' <name>(optional) RETURN 'w'|'b'|'c'|'e'|'g'|'l'|'m'|'p'|'q'|'s'|'t'|'u'|'x'\n");
  printf ("   'qmov' <name> RETURN 'w'|'p'|'m'|'u'\n");
  printf ("   'qmsh' RETURN 'f'|'v'|'n'|'l'|'c'|'s'|'m'|'d'|'q'|'x'|'1'-'9'|' '\n");
  printf ("   'qnod' RETURN 'w'|'p'|'m'|'u'\n");
  printf ("   'qnor' RETURN 'w'|'p'\n");
  printf ("   'qpnt' <name>(optional)RETURN 'w'|'p'|'g'|'m'|'n'|'s'|'S'|'u'\n");
  printf ("   'qrem' <set> RETURN 'w'|'a'|'i'|'r'|'n'|'e'|'f'|'p'|'l'|'s'|'b'|'L'|'S'|'h'|'q'|'u'\n");
  printf ("   'qseq' [[<set>] RETURN 'p']|[ RETURN [<nr>|'l'|'g']]\n");
  printf ("   'qshp' RETURN 'w'|'p'|'g'|'s'|'S'|'h'|'c'|'q'\n");
  printf ("   'qspl' RETURN 'w'|'s'|'q'\n");
  printf ("   'qsur' <name>(optional) RETURN 'w'|'a'|'b'|'l'|'h'|'c'|'i'|'r'|'1'-'9'|'g'|'q'|'u'|'s'|'S'\n");
  printf ("   'qtxt' RETURN 'g'|'a'|'i'|'b'|'p'|'m'|'n'|'v'|'f'|'d'|'q'\n");
  printf ("   'quit' \n");
  printf ("   'read' [<textfile> 'stack']|[<command-file> ['add']]|[<ccx-file> 'inp' ['add'|'ext'|'nom']]|[<ng-file> 'ng' ['add'|'ext'|'nom'|'ndsb']]|[<result-file> ['add'|'ext'|'new'|'nom'|<setname>]]|[<edge-file> 'edg']|[<list-file> '-n'|'-e'[<column>]]|[<picture.xwd> [<zoom>]|[<x_zoom> <y_zoom>]\n");
  printf ("   'rep'  \n");
  printf ("   'rnam' <set> <set> \n");
  printf ("   'rot'  ['n' <set>|'nodenr'] | [['u'|'d'|'r'|'l'|'c' <angle>]|['x'|'-x'|'y'|'-y'|'z'|'-z']]\n");
  printf ("   'save' \n");
  printf ("   'scal' ['s'|'v'|'d'] [<value>]\n");
  printf ("   'send' 'init'|[<set> ['abq'|'adh'|'ans'|'ast'|'bp'|'dar'|'dyn'|'fbd'|'frd'|'gmp'|'lst'|'nas'|'ng'|'pat'|'seq'|'skv'|'stl'|'tcg'] []|['c'|'f'|'e']|['bin']|['dbin']|['comp']|['quadlin']|['names']|['raw']|['nor']|[ ['spc'|'spcf'] [<1-6|t|p> [<value>|['f' <value>]]|['ds'<nr>['+'] e<nr>]]]|['nor' <fac1> [<fac2> <fac3>]]|['slide'|'slidef' 'c'|'s'|'rx'|'ry'|'rz'|'tx'|'ty'|'tz']|['sur' [+|-]]|['pres' [<value>]|['ds'<nr> 'e'<nr>] [+|-]]|['trac' [<v1> <v2> <v3>]|['ds'<nr> 'e'<nr>(1.comp of a vec)] [+|-]]|['ds'<nr> 'e'<nr>] [+|-]]|['force' <f1> <f2> <f3> ]|['film' [[<nodenr>]|[<temp>]|[['ds'<nr>]|[sq<dsnr>-<dsnr>]] 'e'<nr>] [[<coeff>]|[['ds'<nr>|[sq<dsnr>-<dsnr>]] 'e'<nr>]] [+|-]]|['rad' [[<temp>]|[cr<temp>]|['ds'<nr> 'e'<nr>]] [[<emis>|['ds'<nr> 'e'<nr>]]] [+|-]]|['dflux'|'mflow' [[<load>]|['ds'<nr> 'e'<nr>]] [+|-]]|['cflux' <load>]|['mpc' [[<rotation>|'v'<node> <v1> <v2> <v3> ]|['n'<node>]]]|['ds' <nr> 'e'<nr>[','|'-'<nr>..]]|['tmf']|['sta' <refspeed>]|['crp' <timefact> <refspeed> <writefreq>]]\n");
  printf ("  OR 'send' <dep-set> <indep-set> 'nas'|'abq'|'ans'|'ids' ['cycmpc'|'cycmpcf' 'px'|'py'|'pz'|'tx'|'ty'|'tz'|'rx'|'ry'|'rz'|'cx'|'cy'|'cz'<segments> 'c'|'u'<NR>]|['areampc' [<1-6|t|p>|<1|2|3,px,py,pz,vx,vy,vz> 'c'|'u'<Nr>|'f'<value>|['slide']|['presfit' ['s'<value>]]]|['gap' <vx> <vy> <vz> <tol> ]\n");
  printf ("  OR 'send' <set> 'foam' [<base-type> <set>]|['cyclic' <set> <set> 'rx'|'ry'|'rz'|'cx'|'cy'|'cz'|<vx,vy,vz>]\n");  
  printf ("  OR 'send' <set> 'isaac'|'duns' [<base-type> <set>]|['periodic' <set>]\n");  
  printf ("   'seqa' <set> [['afte'|'befo' <name>]|['end']] 'n'|'p' <name> <name>..] \n");
  printf ("   'seqc' <set>\n");
  printf ("   'seql' <set> <nr>\n");
  printf ("   'seta' <set|'!'> ['v'|'n'|'e'|'p'|'l'|'ld'|'c'|'s'|'b'|'S'|'L'|'se'|'sh' <[\\]name|*chars*> ..]|['n'|'e' <name> '-' <name> <steps>]\n");
  printf ("   'setc' <set> \n");
  printf ("   'sete' <set> 'n'|'e'|'p'|'l'|'c'|'s'|'b'|'S'|'L'|'se'|'sh' 'max'|'min'|'strict'\n");
  printf ("   'seti' <set> 'n'|'e'|'p'|'l'|'c'|'s'|'b'|'S'|'L'|'se'|'sh' <set> <set>.. \n");
  printf ("   'setl' <set> ['l']|'u'\n");
  printf ("   'seto' <set> \n");
  printf ("   'setr' <set> 'v'|'n'|'e'|'p'|'l'|'ll'|'la'|'ls'|'ln'|'s'|'b'|'S'|'L'|'se'|'sh' <[\\]name|*chars*> .. \n");
  printf ("   'shpe' <name|!> ['pln' <P1> <P2> <P3>] |['cyl' <P1> <P2> <R1>] |['con' <P1> <P2> <R1> <R2>] |['tor' <P1> <P2> <R1> <R2>] |['sph'] <P1> <R1>]\n");
  printf ("   'split' [[<set>(lines,surfs,tets) <set>(surfs,shape)] ['nos(eparation)'|'e']]|[<line> <point>]\n");
  printf ("   'stack' on|off|free (used by area,dist,ds,enq,gtol,length,prnt,valu,volu,..)\n");
  printf ("   'steps' <value> \n");
  printf ("   'stop' \n");
  printf ("   'subm' <submenuName> <name> <command>\n");
  printf ("   'surf' <name|!> [<set>]|[[<shape>|<nurs>|'blend'] <line|lcmb> <line|lcmb> <line|lcmb> [<line|lcmb>] [<line|lcmb>]] \n");
  printf ("   'swep' <set> <new_set> ['scal' <fx> [<fy> <fz>] [<pnt>|<div>]]|['tra' <x> <y> <z> [<div>]]|['rad' [<p1> <p2> <value> [<div>]]|['x'|'y'|'z' <value> [<div>]]|[<p1> 'x'|'y'|'z' <value> [<div>]]|['p'<PNT> <dr> [<div>]]]|['rot' [<p1> <p2> <value> [<div>]]|['x'|'y'|'z' <value> [<div>]]|[<p1> 'x'|'y'|'z' <value> [<div>]]]|['mir' [<p1> <p2> [<div>]]|['x'|'y'|'z' [<div>]]|[<p1> 'x'|'y'|'z' [<div>]]]|['nor' <value> [<div>]] ['a'[rot:&|'n']]\n");
  printf ("   'sys' <shell-command parameters> \n");
  printf ("   'test' ['file' <file> [<\">string<\">]] | ['i' <set> <set>] | ['o' <set>|'nodenr'] | ['d'|'v'|'n'|'e'|'p'|'l'|'c'|'s'|'b'|'S'|'L'|'se'|'sh' <name>]\n");
  printf ("   'thrs' <set> <value> 'h'|'l'|'o' ['t']\n");
  printf ("   'tra' 'f'|'u'|'d'|'l'|'r' <relative-distance> \n");
  printf ("   'trfm' 'rec'|'cyl' ['x'|'y'|'z'] [<first-Dataset-Nr> [<last-Dataset-Nr>]] (Datasets of a common type)\n");
  printf ("   'txt' '!'|<set>|<node> ['n'|&'v'|&'e'|&'t'|&'f'|&'i'|&'l'|&'r'] [<x> <y>] [<\"text\">]\n");
  printf ("   'typs' \n");
  printf ("   'ucut' \n");
  printf ("   'ulin' <string>\n");
  printf ("   'val' (same as 'valu' but all un-masked <name> are substituted by its value)\n");   
  printf ("   'valu' <[!]name> [['push' [<splitkey>]]|['pop' [nr]]] | [<value> ['?' [<\"string\">]] |  ['&'|'*'|'/'|'+'|'-'|'abs'|'len'|'max'|'min'|'pow'|'sqr'|'log'|'log10'|'sin'|'cos'|'tan'|'asin'|'acos'|'atan'|'int'|'float'|'exp' [name|<const> name|<const>]] ]\n");
  printf ("   'view' ['cl' ['off']]|'fill'|'line'|['point' <value>]|['edge' ['off'|<value>]]|['elem' ['off']]|'surf'|'volu'|'front'|'back'|['vec' ['off']]|['disp' ['off'|'keep']]|['bg' ['w'|'k']]|['sh' ['off']]|['ill' ['off']]|['rul' ['off'|<string>]]\n");
  printf ("   'volu' <set>\n");
  printf ("   'while' <value> 'eq'|'ne'||'=='|'!='|'<'|'>' <value>\n");
  printf ("   'wpos' <xp> <yp>\n");
  printf ("   'wsize' [['f']|[<xp> <yp>]]\n");
  printf ("   'zap'  <set> \n");
  printf ("   'zoom' [<scale>]|[<p1x> <p1y> <p2x> <p2y>] \n");
}


int compareInt(int *a, int *b)
{
  /* wird von qsort aufgerufen, vergleicht Integer-Felder */

  if ( a[0] < b[0] )
    return (-1) ;
  else if ( a[0] > b[0] )
    return (1) ;
  else
    {
    return (0) ;
    }
}

int compareFloat(double *a, double *b)
{
  /* wird von qsort aufgerufen, vergleicht Float-Felder */

  if ( a[0] < b[0] )
    return (-1) ;
  else if ( a[0] > b[0] )
    return (1) ;
  else
    {
    return (0) ;
    }
}

int compareRsort(Rsort *a, Rsort *b)
{
  /* wird von qsort aufgerufen, vergleicht Float-Felder */

  if ( a[0].r < b[0].r )
    return (-1) ;
  else if ( a[0].r > b[0].r )
    return (1) ;
  else
    {
    return (0) ;
    }
}


int pre_readfrdblock( CopiedNodeSets *copiedNodeSets, int lc, Summen *anz,   Nodes     *node, Datasets *lcase )
{
  if( readfrdblock(lc, anz, node, lcase )==-1)
  {
    //printf("ERROR in pre_readfrdblock: Could not read data for Dataset:%d\n", lc+1); 
    return(-1);
  }
  descalNodes ( anz->n, node, scale );
  copyDatasetToNodes(anz, node, lcase, lc, copiedNodeSets[0]);
  scalNodes ( anz->n, node, scale );
  return(1);
}



void resetScaleValues( Scale *scale)
{
  scale->x=scale->y=scale->z=0.; scale->w=1.;
  scale->xmax=1.; scale->xmin=-1.;
  scale->ymax=1.; scale->ymin=-1.;
  scale->zmax=1.; scale->zmin=-1.;
}



void getScaleValues( int setNr, Sets *set, Points *point, Nodes *node, Scale *scale)
{
  int  i,j;
  /* -----------  MAX und MIN Werte feststellen -----------------  */
  if(set[setNr].name == (char *)NULL ) return;

  scale->xmax=-MAX_FLOAT; scale->xmin=MAX_FLOAT;
  scale->ymax=-MAX_FLOAT; scale->ymin=MAX_FLOAT;
  scale->zmax=-MAX_FLOAT; scale->zmin=MAX_FLOAT;

  for (j=0; j<set[setNr].anz_n; j++ )
  {
    i=set[setNr].node[j];
    if(node[i].pflag==0)
    {
    if (node[i].nx > scale->xmax) scale->xmax=node[i].nx;
    if (node[i].nx < scale->xmin) scale->xmin=node[i].nx;
    if (node[i].ny > scale->ymax) scale->ymax=node[i].ny;
    if (node[i].ny < scale->ymin) scale->ymin=node[i].ny;
    if (node[i].nz > scale->zmax) scale->zmax=node[i].nz;
    if (node[i].nz < scale->zmin) scale->zmin=node[i].nz;
    }
  }
  for (j=0; j<set[setNr].anz_p; j++ )
  {
    i=set[setNr].pnt[j];
    if(point[i].name!=NULL)
    {
    if (point[i].px > scale->xmax) scale->xmax=point[i].px;
    if (point[i].px < scale->xmin) scale->xmin=point[i].px;
    if (point[i].py > scale->ymax) scale->ymax=point[i].py;
    if (point[i].py < scale->ymin) scale->ymin=point[i].py;
    if (point[i].pz > scale->zmax) scale->zmax=point[i].pz;
    if (point[i].pz < scale->zmin) scale->zmin=point[i].pz;
    }
  }

  /* ------------  DATEN scalieren -------------------  */

  scale->x=(scale->xmax+scale->xmin)/2.;
  scale->y=(scale->ymax+scale->ymin)/2.;
  scale->z=(scale->zmax+scale->zmin)/2.;
  scale->xmax=scale->xmax-scale->x;
  scale->ymax=scale->ymax-scale->y;
  scale->zmax=scale->zmax-scale->z;
  scale->xmin=scale->xmin-scale->x;
  scale->ymin=scale->ymin-scale->y;
  scale->zmin=scale->zmin-scale->z;
  if (scale->xmax < (-scale->xmin)) scale->xmax=(-scale->xmin);
  if (scale->ymax < (-scale->ymin)) scale->ymax=(-scale->ymin);
  if (scale->zmax < (-scale->zmin)) scale->zmax=(-scale->zmin);
  scale->w=scale->xmax;
  if (scale->w < scale->ymax){ scale->w=scale->ymax;}
  if (scale->w < scale->zmax){ scale->w=scale->zmax;}

  scale->w/=0.4; /* nochmal scaliert */
  if (scale->w<=0.) scale->w=1.;
  //printf("scale: %f %f %f %f\n", scale->x, scale->y, scale->z, scale->w);
}


void defineColTextur_load(float alpha)
{
  int   i, n, buf;
  float r, g, b;
  extern GLfloat   *contur_tex;
  GLuint tex_id;

  /* define colormap values in both colormaps */
  if( (contur_tex= (GLfloat *)realloc( (GLfloat *)contur_tex, ((TEX_PIXELS+1)*4)*sizeof(GLfloat ) ))==NULL )
    printf("ERROR: realloc failed: defineColTextur_load \n\n" );

  if(steps>TEX_PIXELS) steps=TEX_PIXELS;

  n=0;
  for (i=0; i<steps; i++)
  {
    buf=i;
    if(!flipColorFlag)
    {
      if(scale->smaxr==2) buf++;
      define_rgb( (float)buf/(steps-1), &r,&g,&b);
    }
    else
    {
      if(scale->sminr==2) buf--;
      define_rgb( (steps-1-(float)buf)/(steps-1.), &r,&g,&b);
    }
    
    if(i>=steps-1 && scale->smaxr==2)
    {
    contur_tex[n]  =entitycol[col_maxc].r;
    contur_tex[n+1]=entitycol[col_maxc].g;
    contur_tex[n+2]=entitycol[col_maxc].b;
    contur_tex[n+3]=alpha;
    }
    else if(i<=0 && scale->sminr==2)
    {
    contur_tex[n]  =entitycol[col_minc].r;
    contur_tex[n+1]=entitycol[col_minc].g;
    contur_tex[n+2]=entitycol[col_minc].b;
    contur_tex[n+3]=alpha;	    
    }
    else
    {
    contur_tex[n]  =r;
    contur_tex[n+1]=g;
    contur_tex[n+2]=b;
    contur_tex[n+3]=alpha;
    }
    // value dependent blending
    //contur_tex[n+3]=alpha+((1-alpha)*(double)i/(double)steps);
    n+=4;
  }
  for (; i<TEX_PIXELS; i++)
  {
    if(i>=steps-1 && scale->smaxr==2)
    {
    contur_tex[n]  =entitycol[col_maxc].r;
    contur_tex[n+1]=entitycol[col_maxc].g;
    contur_tex[n+2]=entitycol[col_maxc].b;
    contur_tex[n+3]=alpha;
    }
    else if(i<=0 && scale->sminr==2)
    {
    contur_tex[n]  =entitycol[col_minc].r;
    contur_tex[n+1]=entitycol[col_minc].g;
    contur_tex[n+2]=entitycol[col_minc].b;
    contur_tex[n+3]=alpha;	    
    }
    else
    {
    contur_tex[n]  =r;
    contur_tex[n+1]=g;
    contur_tex[n+2]=b;
    contur_tex[n+3]=alpha;
    n+=4;
    }
  }

  /*
  n=0;
  for (i=0; i<TEX_PIXELS; i++)
  {    printf("%d %d %lf %lf %lf %lf\n",steps, i, contur_tex[n], contur_tex[n+1], contur_tex[n+2], contur_tex[n+3]); n+=4; }
  */
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_1D, tex_id);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage1D(GL_TEXTURE_1D, 0, 4, TEX_PIXELS, 0, GL_RGBA, GL_FLOAT, contur_tex);
}



void rot_u(double a)
{
  int i,n;
  double b;

    b=a/90.; i=b;
    a=a*PI/180.;
    if(i<0) for(n=0; n<-i; n++)
    {  
      trackball( 1, trackbsize, curquat, 0.0, 0.0, 0.0, trackbsize );
      add_quats(curquat, lastquat, lastquat);
      build_rotmatrix( R, lastquat );
    }
    if(i>0) for(n=0; n<i; n++)
    {    
      trackball( 1, trackbsize, curquat, 0.0, 0.0, 0.0, -trackbsize );
      add_quats(curquat, lastquat, lastquat);
      build_rotmatrix( R, lastquat );
    }
    a=(b-i)*PI/2.;
    trackball( 1, trackbsize, curquat, 0.0, 0.0, 0.0, -sin(a)*trackbsize );
    add_quats(curquat, lastquat, lastquat);
    build_rotmatrix( R, lastquat );
}
void rot_r(double a)
{
  int i,n;
  double b;

    b=-a/90.; i=b;
    a=-a*PI/180.;
    if(i<0) for(n=0; n<-i; n++)
    {    
      trackball( 1, trackbsize, curquat, 0.0, 0.0, -trackbsize, 0.0 );
      add_quats(curquat, lastquat, lastquat);
      build_rotmatrix( R, lastquat );
    }
    if(i>0) for(n=0; n<i; n++)
    {    
      trackball( 1, trackbsize, curquat, 0.0, 0.0, trackbsize, 0.0 );
      add_quats(curquat, lastquat, lastquat);
      build_rotmatrix( R, lastquat );
    }
    a=(b-i)*PI/2.;
    trackball( 1, trackbsize, curquat, 0.0, 0.0, sin(a)*trackbsize, 0.0 );
    add_quats(curquat, lastquat, lastquat);
    build_rotmatrix( R, lastquat );
}
void rot_c(double a)
{
  int i,n;
  double b;

    b=a/90.; i=b;
    a=a*PI/180.;
    if(i<0) for(n=0; n<-i; n++)
    {    
      trackball( 1, trackbsize, curquat, trackbsize, 0.0, 0.0, -trackbsize );
      add_quats(curquat, lastquat, lastquat);
      build_rotmatrix( R, lastquat );
    }
    if(i>0) for(n=0; n<i; n++)
    {    
      trackball( 1, trackbsize, curquat, trackbsize, 0.0, 0.0, trackbsize );
      add_quats(curquat, lastquat, lastquat);
      build_rotmatrix( R, lastquat );
    }
    a=(b-i)*PI/2.;
    trackball( 1, trackbsize, curquat, trackbsize, 0.0, cos(a)*trackbsize, sin(a)*trackbsize );
    add_quats(curquat, lastquat, lastquat);
    build_rotmatrix( R, lastquat );
}

void rot_x(double a)
{
    /* Trackballfunktion inizialisieren, Blickrichtung +x */
    a*=90.*PI/180.;
    trackball( 1, trackbsize, lastquat, 0.0, 0.0, sin(a)*trackbsize, 0.0 );
    R[0][0]=cos(a);  R[1][0]=0.;  R[2][0]=sin(a);  R[3][0]=0.;
    R[0][1]=0.;  R[1][1]=1.;  R[2][1]=0.;  R[3][1]=0.;
    R[0][2]=-sin(a); R[1][2]=0.;  R[2][2]=cos(a);  R[3][2]=0.;
    R[0][3]=0.;  R[1][3]=0.;  R[2][3]=0.;  R[3][3]=1.;
}

void rot_y(double a)
{
    /* Trackballfunktion inizialisieren, Blickrichtung +y (rot_x)*/
    a*=-90.*PI/180.;
    trackball( 1, trackbsize, lastquat, 0.0, 0.0, 0.0, -sin(a)*trackbsize );
    R[0][0]=1.;  R[1][0]=0.;  R[2][0]=0.;  R[3][0]=0.;
    R[0][1]=0.;  R[1][1]=cos(a);  R[2][1]=-sin(a);  R[3][1]=0.;
    R[0][2]=0.;  R[1][2]=sin(a);  R[2][2]=cos(a);  R[3][2]=0.;
    R[0][3]=0.;  R[1][3]=0.;  R[2][3]=0.;  R[3][3]=1.;
}

void rot_z(double b)
{
  double a;
    /* Trackballfunktion inizialisieren, Blickrichtung +z (rot_z)*/
    a=0.;
    trackball( 1, trackbsize, lastquat, trackbsize, 0.0, cos(a)*trackbsize, sin(a)*trackbsize );
    R[0][0]=cos(a);  R[1][0]=-sin(a);  R[2][0]=0.;  R[3][0]=0.;
    R[0][1]=sin(a);  R[1][1]=cos(a);   R[2][1]=0.;  R[3][1]=0.;
    R[0][2]=0.;  R[1][2]=0.;  R[2][2]=1.;  R[3][2]=0.;
    R[0][3]=0.;  R[1][3]=0.;  R[2][3]=0.;  R[3][3]=1.;
    if(b==1) rot_r(180.);
}

int rot_norm(int nr)
{
  int    setcopy;
  static int *sum_n=NULL;
  static Nodes *norm=NULL;

  static double p03[3], p03xz[3];
  double  ay, axz;
  double  p03_b, p03xz_b, scalar, sig;

  /* determine the normal based on all connected faces */
  delSet("+norm");
  setcopy=pre_seta( "+norm", "i", 0);
  if (setcopy<0)
  {
    errMsg (" ERROR: set:+norm could not be created\n" );
    return(1);
  }
  seta( setcopy, "n", nr );
  completeSet("+norm", "up") ;
  completeSet("+norm", "do") ;
  getNodeNormalen(&sum_n, &norm, setcopy, anz, face);
  delSet("+norm");

  p03[0]=norm[nr].nx;
  p03[1]=norm[nr].ny;
  p03[2]=norm[nr].nz;
  if(v_betrag(p03)<0.5) return(2); 

  center( node[nr].nx, node[nr].ny, node[nr].nz ); 

  /* drehe den Ort des Betrachters  */
  ay=atan(p03[0]/p03[2]);

  p03xz[0]=p03[0];
  p03xz[1]=0.;
  p03xz[2]=p03[2];
  scalar = v_sprod( p03xz, p03 );
  p03xz_b= v_betrag( p03xz );
  p03_b  = v_betrag( p03 );
  axz =acos( scalar/p03xz_b/p03_b ) * 180./PI;

  /* vorzeichen von der y-Komponente der Normalen wird zum vorzeichen von axz */
  scalar= sqrt(p03[1]*p03[1]);
  sig = p03[1]/scalar;
  axz = axz*sig;
  ay=ay*180./PI;

  /* Richtungsumkehr wenn die z-Komponente der Normalen nach hinten zeigt (-) */
  if (p03[2]<0)
  {
    rot_z(1.);
  }
  else rot_z(-1.);
  if((ay>=-360.)&&(ay<=360.)) rot_r( ay );
  if((axz>=-360.)&&(axz<=360.)) rot_u( axz );
  return(0);
}



void transformResults( char *record )
{
  int i;
  char method[MAX_LINE_LENGTH], axis[MAX_LINE_LENGTH];
  int lcmin=0, lcmax=0;
  int lc, *dsNr=NULL, anz_lc=0;

  if(!anz->l) return;

  sscanf(record,"%s %s %d %d", method, axis, &lcmin, &lcmax);
  lcmin--; lcmax--;
  if(lcmin<0) { lcmin=lcmax=cur_lc; }
  if(lcmax<0) lcmax=lcmin;
  if(lcmin>anz->olc) lcmin=anz->olc;
  if(lcmax>anz->olc) lcmax=anz->olc;

  /* get the list of datasets */
  for(lc=lcmin; lc<=lcmax; lc++)
  {
    //if( compare( lcase[lc].name, dataset, strlen(lcase[lcmin].name)) == strlen(lcase[lcmin].name) )
    if(compareStrings(lcase[lc].name, lcase[lcmin].name)>0)
    {
      anz_lc++; 
      if((dsNr=(int *)realloc((int *)dsNr, (anz_lc+2) *sizeof(int)))==NULL )
        printf("\n\n ERROR: malloc failed \n\n") ;
      dsNr[anz_lc]=lc;
    }
  }
  if(!anz_lc)
  {
    printf(" ERROR: found no matching Dataset for string:%s\n", lcase[lcmin].name);
    return;
  }
  dsNr[0]=anz_lc;

  /* check if the data of the specified lcase (Dataset) are already available */
  printf (" please wait, loading data\n");
  for(i=1; i<=dsNr[0]; i++)
  {
    lc=dsNr[i];
    if (!lcase[lc].loaded)
    {
      if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
      {
        printf("ERROR in transformResults: Could not read data for Dataset:%d\n", lc+1); 
        return;
      }
      calcDatasets( lc, anz, node, lcase );
      recompileEntitiesInMenu(lc);
    }

    /* transform dataset */
    printf("transform dataset nr:%d \n", lc+1);
    transformDatasets( lc, anz, node, lcase, method, axis );
    recompileEntitiesInMenu(lc);
  }
}



/* transformiert punkte-koordinaten und liefert die methode der transformation zurueck */
/*
   fehler: -1
   tra: 1
   rad: 2
   rot: 3
   sca: 4
   mir: 5
   equ: 6
   nor: 7
*/
int transform( char *record, int anz_n, Nodes *nslave )
{
  int  length, i,j;
  char v[MAX_LINE_LENGTH], dat[4][MAX_LINE_LENGTH];
  char type[MAX_LINE_LENGTH], pkt1[MAX_LINE_LENGTH], pkt2[MAX_LINE_LENGTH];
  int   pnr1, pnr2, setNr;
  double fi=0., dx=0., dy=0., dz=0., dr=0., x, y, z, r, dh, dq, l, h, q, h_offs, q_offs, tol;
  double val[3]={0,0,0}, xdr1, xdr2, dr1, dr2;
  double s[4]={0,0,0,0}, p1[3], p2[3], p1p2[3];
  double ph[3], p1ph[3];
  double el[3], eh[3], eq[3];   /* xyz koordinaten der lhq vektoren */
  double ex[3]={1.,0.,0.}, ey[3]={0.,1.,0.}, ez[3]={0.,0.,1.};   /* lhq koordinaten der xyz vektoren */
  double sprod;
  double ep1p2[3],p1n0[3],p1n1[3],n0[3],n1[3],n1n0[3],en1n0[3],n1n2[3],n2[3];
  static Nodes *norm=NULL;
  static int *sum_n=NULL;

  length = sword( record, type );

  if(( compare(type,"tra",3) == 3 )||( compare(type,"TRA",3) == 3 ))
  {
    sscanf( record, "%*s %lf %lf %lf", &dx, &dy, &dz );
    for (i=0; i<anz_n; i++)
    {
      nslave[i].nx+= dx;
      nslave[i].ny+= dy;
      nslave[i].nz+= dz;
    }
    return(1);
  }
  else if(( compare(type,"nor",3) == 3 )||( compare(type,"NOR",3) == 3 ))
  {
    // works only for nodes
    sscanf( record, "%*s %lf", &dr );

    /* determine the normal based on all connected faces, code from pre_norm()  */
    delSet("+norm");
    setNr=pre_seta( "+norm", "i", 0);
    if (setNr<0)
    {
      errMsg (" ERROR: set:+norm could not be created\n" );
      return(-1);
    }
    for (i=0; i<anz_n; i++)
    {
      seta( setNr, "n", nslave[i].indx  );
    }
    completeSet("+norm", "do") ;
    getNodeNormalen(&sum_n, &norm, setNr, anz, face);
    delSet("+norm");
    for (i=0; i<anz_n; i++)
    {
      nslave[i].nx+= norm[nslave[i].indx].nx*dr;
      nslave[i].ny+= norm[nslave[i].indx].ny*dr;
      nslave[i].nz+= norm[nslave[i].indx].nz*dr;
    }
    return(7);
  }
  else if(( compare(type,"equ",3) == 3 )||( compare(type,"EQU",3) == 3 ))
  {
    tol=gtol;
    sscanf( record, "%*s %s %lf", dat[0], &tol );
    setNr=getSetNr( dat[0] );
    if (setNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", dat[0] );
      return(-1);
    }
    if ((set[setNr].anz_p<1)&&(set[setNr].anz_n<1))
    {
      errMsg(" ERROR: Set (%s) holds no target-nodes or points\n", dat[0] );
      return(-1);
    }

    for(j=0; j<set[setNr].anz_p; j++)
    {
      pnr1=set[setNr].pnt[j];
      for (i=0; i<anz_n; i++)
      {
        dx= point[pnr1].px*scale->w+scale->x-nslave[i].nx;
        dy= point[pnr1].py*scale->w+scale->y-nslave[i].ny;
        dz= point[pnr1].pz*scale->w+scale->z-nslave[i].nz;
	//printf("dr=%f tol:%f\n", sqrt(dx*dx+dy*dy+dz*dz), tol);
        if(sqrt(dx*dx+dy*dy+dz*dz) <= tol)
	{
          nslave[i].nx=point[pnr1].px*scale->w+scale->x;
          nslave[i].ny=point[pnr1].py*scale->w+scale->y;
          nslave[i].nz=point[pnr1].pz*scale->w+scale->z;
	}
      }
    }

    for(j=0; j<set[setNr].anz_n; j++)
    {
      pnr1=set[setNr].node[j];
      for (i=0; i<anz_n; i++)
      {
        dx= node[pnr1].nx*scale->w+scale->x-nslave[i].nx;
        dy= node[pnr1].ny*scale->w+scale->y-nslave[i].ny;
        dz= node[pnr1].nz*scale->w+scale->z-nslave[i].nz;
	//printf("dr=%f tol:%f\n", sqrt(dx*dx+dy*dy+dz*dz), tol);
        if(sqrt(dx*dx+dy*dy+dz*dz) <= tol)
	{
          nslave[i].nx=node[pnr1].nx*scale->w+scale->x;
          nslave[i].ny=node[pnr1].ny*scale->w+scale->y;
          nslave[i].nz=node[pnr1].nz*scale->w+scale->z;
	}
      }
    }
    return(6);
  }
  else if(( compare(type,"sca",3) == 3 )||( compare(type,"SCA",3) == 3 ))
  {
    length=sscanf( record, "%*s %s %s %s %s", dat[0], dat[1], dat[2], dat[3] );
    for(i=0; i<length; i++) { s[i]=atof(dat[i]);  }

    pnr1=getPntNr(dat[length-1]);
    if (pnr1<0)
    {
      /* check if we have a valid number */
      if (s[length-1]==0.)
      {
        errMsg(" Argument:%s is not valid\n", dat[length-1] );
        return(-1);
      }
    }
    else length--;
    
    if (length==0) s[1]=s[2]=s[0]=1.;
    if (length==1) s[1]=s[2]=s[0];
    if (length==2) s[2]=1.;
    if (pnr1>-1)
    {
      ph[0]=(point[pnr1].px*scale->w+scale->x);
      ph[1]=(point[pnr1].py*scale->w+scale->y);
      ph[2]=(point[pnr1].pz*scale->w+scale->z);
    }
    else ph[0]=ph[1]=ph[2]=0.;                                                    

    for (i=0; i<anz_n; i++)
    {
      nslave[i].nx= ph[0] - s[0]*(ph[0]-nslave[i].nx);
      nslave[i].ny= ph[1] - s[1]*(ph[1]-nslave[i].ny);
      nslave[i].nz= ph[2] - s[2]*(ph[2]-nslave[i].nz);
    }
    return(4);
  }
  else if(( compare(type,"rad",3) == 3 )||( compare(type,"RAD",3) == 3 ))
  {
    length=sscanf( record, "%*s%s%s%lf%lf%lf", pkt1, pkt2, &val[0], &val[1], &val[2] );

    if(checkIfNumber(pkt2))
    {
      strcpy(v,pkt1);
      if (v[0]=='p')
      {
        strcpy(pkt1, &v[1]);
        pnr1=getPntNr( pkt1 );
        if (pnr1==-1)
        {
          errMsg(" ERROR: Point (%s) is undefined\n", pkt1 );
          return(-1);
        }
        ph[0]=(point[pnr1].px*scale->w+scale->x);
        ph[1]=(point[pnr1].py*scale->w+scale->y);
        ph[2]=(point[pnr1].pz*scale->w+scale->z);
      }
  
      for (i=0; i<anz_n; i++)
      {
        x=nslave[i].nx;
        y=nslave[i].ny;
        z=nslave[i].nz;

        if(length==5)
	{
          dr1=atof(pkt2);
          xdr1=val[0];
          dr2=val[1];
          xdr2=val[2];
          if (v[0]=='x') dr=(x-xdr1)/(xdr2-xdr1)*(dr2-dr1) + dr1;
          if (v[0]=='y') dr=(y-xdr1)/(xdr2-xdr1)*(dr2-dr1) + dr1;
          if (v[0]=='z') dr=(z-xdr1)/(xdr2-xdr1)*(dr2-dr1) + dr1;
	}
	else dr=atof(pkt2);

        if (v[0]=='x')
        {
          r=sqrt(y*y+z*z);
          if (r>gtol)
          {
            y=y*(r+dr)/r;
            z=z*(r+dr)/r;
          }
        }
        else if (v[0]=='y')
        {
          r=sqrt(x*x+z*z);
          if (r>gtol)
          {
            x=x*(r+dr)/r;
            z=z*(r+dr)/r;
          }
        }
        else if (v[0]=='z')
        {
          r=sqrt(y*y+x*x);
          if (r>gtol)
          {
            y=y*(r+dr)/r;
            x=x*(r+dr)/r;
          }
        }
        else if (v[0]=='p')
        {
          dx=x-ph[0];
          dy=y-ph[1];
          dz=z-ph[2];
          r=sqrt(dy*dy+dx*dx+dz*dz);
          if (r>gtol)
          {
            x=ph[0]+dx*(r+dr)/r;
            y=ph[1]+dy*(r+dr)/r;
            z=ph[2]+dz*(r+dr)/r;
          }
        }
        else
        { errMsg(" rotational Axis not known:%1s \n", v); return(-1); }
        nslave[i].nx= x;
        nslave[i].ny= y;
        nslave[i].nz= z;
      }
    }
    else
    {
      dr=val[0];
      pnr1=getPntNr( pkt1 );
      pnr2=getPntNr( pkt2 );
      if (pnr1==-1)
      {
        errMsg(" ERROR: Point1 (%s) is undefined\n", pkt1 );
        return(-1);
      }
      else
      {
        p1[0] = point[pnr1].px*scale->w+scale->x;
        p1[1] = point[pnr1].py*scale->w+scale->y;
        p1[2] = point[pnr1].pz*scale->w+scale->z;
      }
      if (pnr2==-1)
      {
        if(compareStrings(pkt2,"x")>0) v_add(p1, ex, p2);
        else if(compareStrings(pkt2,"y")>0) v_add(p1, ey, p2);
        else if(compareStrings(pkt2,"z")>0) v_add(p1, ez, p2);
        else { errMsg(" ERROR: Point2 (%s) is undefined\n", pkt2 );        return(-1); }
      }
      else
      {
        p2[0] = point[pnr2].px*scale->w+scale->x;
        p2[1] = point[pnr2].py*scale->w+scale->y;
        p2[2] = point[pnr2].pz*scale->w+scale->z;
      }
      v_result(p1,p2,p1p2);

      for (i=0; i<anz_n; i++)
      {
        n0[0]=nslave[i].nx;
        n0[1]=nslave[i].ny;
        n0[2]=nslave[i].nz;
        v_result(p1,n0,p1n0);
        if(v_betrag(p1n0)<1.e-20) continue;

        /* projection of nslave onto p1p2 with: unit-vector * scalar-product */
        v_norm(p1p2, ep1p2);
        sprod=v_sprod(ep1p2,p1n0);
        v_scal(&sprod,ep1p2, p1n1);

        /* projection point n1 */
        v_add(p1,p1n1,n1); 

        /* unit vector to radial moved point n2 */
        v_result(n1,n0,n1n0);
        v_norm(n1n0, en1n0);

        /* distance from n1 to n2 */
        l=v_betrag(n1n0);

        /* do not move locations which are very close to the axis */
        if(l>gtol) l+=dr;
        //else printf(" i:%d to close to cl to be moved radially\n",i);

        /* vector from n1 to n2 */
        v_scal(&l,en1n0, n1n2);

        /* point n2 */
        v_add(n1,n1n2,n2); 
        
        nslave[i].nx= n2[0];
        nslave[i].ny= n2[1];
        nslave[i].nz= n2[2];
      }
    }
    return(2);
  }
  else if(( compare(type,"rot",3) == 3 )||( compare(type,"ROT",3) == 3 ))
  {
    length=sscanf( record, "%*s%s%s%lf%lf%lf", pkt1, pkt2, &val[0], &val[1], &val[2] );
    i=sscanf( record, "%*s%s%s%lf", pkt1, pkt2, &fi );

    if(checkIfNumber(pkt2))
    {
      strcpy(v,pkt1);

      for (i=0; i<anz_n; i++)
      {
        x=nslave[i].nx;
        y=nslave[i].ny;
        z=nslave[i].nz;

        if(length==5)
	{
          dr1=atof(pkt2);
          xdr1=val[0];
          dr2=val[1];
          xdr2=val[2];
          if (v[0]=='x') dr=(x-xdr1)/(xdr2-xdr1)*(dr2-dr1) + dr1;
          if (v[0]=='y') dr=(y-xdr1)/(xdr2-xdr1)*(dr2-dr1) + dr1;
          if (v[0]=='z') dr=(z-xdr1)/(xdr2-xdr1)*(dr2-dr1) + dr1;
	  fi=dr*PI/180.;
	}
	else fi=atof(pkt2)*PI/180.;

        if (v[0]=='x')
        {
          nslave[i].ny=y*cos(fi)-z*sin(fi);
          nslave[i].nz=y*sin(fi)+z*cos(fi);
        }
        else if (v[0]=='y')
        {
          nslave[i].nz=z*cos(fi)-x*sin(fi);
          nslave[i].nx=z*sin(fi)+x*cos(fi);
        }
        else if (v[0]=='z')
        {
          nslave[i].nx=x*cos(fi)-y*sin(fi);
          nslave[i].ny=x*sin(fi)+y*cos(fi);
        }
        else
        { errMsg(" rotational Axis not known:%1s \n", v); return(-1); }
      }
    }
    else
    {
      fi = val[0]*PI/180.;
      pnr1=getPntNr( pkt1 );
      pnr2=getPntNr( pkt2 );
      if (pnr1<0)
      {
        errMsg(" ERROR: Point1 (%s) is undefined\n", pkt1 );
        return(-1);
      }
      else
      {
        p1[0] = point[pnr1].px;
        p1[1] = point[pnr1].py;
        p1[2] = point[pnr1].pz;
      }
      if (pnr2<0)
      {
        if(compareStrings(pkt2,"x")>0) v_add(p1, ex, p2);
        else if(compareStrings(pkt2,"y")>0) v_add(p1, ey, p2);
        else if(compareStrings(pkt2,"z")>0) v_add(p1, ez, p2);
        else { errMsg(" ERROR: Point2 (%s) is undefined\n", pkt2 );        return(-1); }
      }
      else
      {
        p2[0] = point[pnr2].px;
        p2[1] = point[pnr2].py;
        p2[2] = point[pnr2].pz;
      }
    
      /* berechnung der Einheitsvektoren des Verdreh-Koordinatensystems */
      /* Exneu = p1p2/|p1p2| ist der Einheitsvektor in xneu richtung    */
      /* Eyneu = p1p2 X p1ph / |p1p2 X p1ph|     in yneu                */
      /* Ezneu = Exneu X Eyneu                   in zneu                */
    
      v_result( p1, p2, p1p2 );
      v_norm  ( p1p2, el );
    
      /* erzeuge einen Hilfspunkt der nicht auf der el-achse liegt  */
      ph[1] = p1p2[0];
      ph[2] = p1p2[1];
      ph[0] = p1p2[2];
    
      /* konstuiere damit den 2. einheitsvektor eh  */
      v_result( p1, ph, p1ph );
      v_prod( p1p2, p1ph, ph );
      v_norm (ph, eh);
    
      /* und der dritte: eq  */
      v_prod( p1p2, ph, ex );
      v_norm (ex, eq);
    
      /* berechnung der lhq-koordinaten der xyz einheitsvektoren durch zykl.vertausch.  */
      ex[0]=el[0];
      ex[1]=eh[0];
      ex[2]=eq[0];
    
      ey[0]=el[1];
      ey[1]=eh[1];
      ey[2]=eq[1];
    
      ez[0]=el[2];
      ez[1]=eh[2];
      ez[2]=eq[2];
    
      /* Berechnung der lhq-koordinaten der Drehachse (offset fuer die Drehung) */
      x=point[pnr1].px*scale->w+scale->x;
      y=point[pnr1].py*scale->w+scale->y;
      z=point[pnr1].pz*scale->w+scale->z;
      
      /* l=ex[0]*x+ey[0]*y+ez[0]*z; */
      h_offs=ex[1]*x+ey[1]*y+ez[1]*z;
      q_offs=ex[2]*x+ey[2]*y+ez[2]*z;
    
      /* Berechnung der lhq-koordinaten aller zu drehenden punkte */
      for (i=0; i<anz_n; i++)
      {
        x=nslave[i].nx;
        y=nslave[i].ny;
        z=nslave[i].nz;
        l=ex[0]*x+ey[0]*y+ez[0]*z;
        h=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;
        q=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;
    
        /* drehe um l  */
        dh=h*cos(fi)-q*sin(fi);
        dq=h*sin(fi)+q*cos(fi);
        dh+= h_offs;
        dq+= q_offs;
    
        nslave[i].nx=el[0]*l+eh[0]*dh+eq[0]*dq;
        nslave[i].ny=el[1]*l+eh[1]*dh+eq[1]*dq;
        nslave[i].nz=el[2]*l+eh[2]*dh+eq[2]*dq;
      }
    }
    return(3);
  }
  else if(( compare(type,"mir",3) == 3 )||( compare(type,"MIR",3) == 3 ))
  {
    /* the "mirror" is placed at p2 and is perpendicular to p1p2 */
    length=sscanf( record, "%*s%s%s", pkt1, pkt2 );
    pnr1=getPntNr( pkt1 );
    if(length==2) pnr2=getPntNr( pkt2 ); else pnr2=-1;
    if (pnr2<0) pnr2=-1;

    if (pnr1<0)
    {
      pnr2=-2;
      p1[0] = 0.;
      p1[1] = 0.;
      p1[2] = 0.;
      if(pkt1[0]=='x')
      {
        v_add(p1, ex, p2);
      }
      else if(pkt1[0]=='y')
      {
        v_add(p1, ey, p2);
      }
      else if(pkt1[0]=='z')
      {
        v_add(p1, ez, p2);
      }
      else { errMsg("x ERROR: Point (%s) is undefined\n", pkt1 ); return(-1); }

      /* switch p1 p2 */
      for(i=0; i<3; i++) { p1p2[i]=p1[i]; p1[i]=p2[i]; p2[i]=p1p2[i]; }
    }
    else
    {
      p1[0] = point[pnr1].px*scale->w+scale->x;
      p1[1] = point[pnr1].py*scale->w+scale->y;
      p1[2] = point[pnr1].pz*scale->w+scale->z;
    }

    if (pnr2==-1)
    {
      if(pkt2[0]=='x')
      {
        v_add(p1, ex, p2);
      }
      else if(pkt2[0]=='y')
      {
        v_add(p1, ey, p2);
      }
      else if(pkt2[0]=='z')
      {
        v_add(p1, ez, p2);
      }
      else { errMsg("y ERROR: Point (%s) is undefined\n", pkt2 ); return(-1); }

      /* switch p1 p2 */
      for(i=0; i<3; i++) { p1p2[i]=p1[i]; p1[i]=p2[i]; p2[i]=p1p2[i]; }
    }
    else if (pnr2>=0)
    {
      p2[0] = point[pnr2].px*scale->w+scale->x;
      p2[1] = point[pnr2].py*scale->w+scale->y;
      p2[2] = point[pnr2].pz*scale->w+scale->z;
    }

    /* calculation of the first mirrored point (modification of p2) */
    v_result( p1, p2, p1p2 );
    l=2.;
    v_scal( &l, p1p2, p1p2 ); 
    v_add( p1,  p1p2, p2 );

    ph[0] = p2[0]+p1[0];
    ph[1] = p2[1]+p1[1];
    ph[2] = p2[2]+p1[2];

    fi=v_sprod(p1p2, p1p2);

    for (i=0; i<anz_n; i++)
    {
      el[0]=(ph[0]-2*nslave[i].nx) ;
      el[1]=(ph[1]-2*nslave[i].ny) ;
      el[2]=(ph[2]-2*nslave[i].nz) ;
      l=v_sprod(el, p1p2);
      h=l/fi;
      nslave[i].nx+= h*p1p2[0];
      nslave[i].ny+= h*p1p2[1];
      nslave[i].nz+= h*p1p2[2];
    }
    return(5);
  }
  else
  {
    printf(" ERROR: transformation not recognized: %s\n", type );
    return(-1);
  }
}

/* calculate the necessary division of the line with respect to the curvature */
int calcLineDiv(Lines *line, int nr, double gtol_cos_a, double lmax, double lmin)
{
  int i,imin=0,imax=0,sig=0;

  double e0[3], e1[3], v0[3], v1[3],bias=1;
  double l, cos_a, min_cos_a=MAX_FLOAT, max_cos_a=0., min_cos_a_buf;

  /* generate the initial drawing and potential meshing points (called points now) on the line */

  /* start with a high div and relax it as long as the requirements are fullfilled */
  line[nr].div=MAX_LINE_DIV-(MAX_LINE_DIV%2);
  line[nr].bias=1.;

  /* change the div until lmax is reached */
 new_div:;
  repLine(nr);
  v_result(&line[nr].ip[0],&line[nr].ip[3], v0); 
  l=v_betrag(v0);
  if(l>=lmax)
  {
    line[nr].div=1+(double)line[nr].div*l/lmax;
    goto new_div;
  }

  /* calculate the angle between vectors from 1st point to second and to 3rd, over all points */
 new_bia:;
  imin=imax=sig=0;
  if (min_cos_a<MAX_FLOAT) min_cos_a_buf=min_cos_a; else min_cos_a_buf=0.;
  for(i=0; i<line[nr].nip-6; i+=3)
  {
    v_result(&line[nr].ip[i],&line[nr].ip[i+3], v0); 
    v_result(&line[nr].ip[i],&line[nr].ip[i+6], v1); 
    v_norm(v0, e0);
    v_norm(v1, e1);
    cos_a= v_sprod(e0, e1);
    if(cos_a<min_cos_a) { min_cos_a=cos_a; imin=i; }
    if(cos_a>max_cos_a) { max_cos_a=cos_a; imax=i; }
  }

  /* change the bia as long as min_cos_a increases and  min_cos_a/max_cos_a gets smaller */
  //if(i) printf(" i:%d cos_a %f %f %f a:%f line[nr].bias:%f \n",i,min_cos_a_buf,min_cos_a,max_cos_a,asin(max_cos_a-min_cos_a)*180.,line[nr].bias );
  if(i)
  {
    //printf("%s max_cos_a-min_cos_a %f g:%f\n",line[nr].name, max_cos_a-min_cos_a,MIN_DCOS_A);
    // is the angle difference significant?
    if((max_cos_a-min_cos_a)>MIN_DCOS_A)
    {
      //printf("imin:%d i/3:%d i*2/3:%d\n", imin,i/3,i*2/3);
      // is the maximum curvature at the borders?
      if((imin<i/3)||(imin>i*2/3))
      {
        //printf("cos_a buf %f %f\n",min_cos_a_buf,min_cos_a);
        // decreases the maximum angle?
        if(min_cos_a>min_cos_a_buf)
	{
          bias*=2;
          if(imin>imax)  // greater curvature at line end, smaller elem at end
          {
            sig=-1;
            line[nr].bias= 1./pow((bias), (1./((double)line[nr].div-1.)));
          }
          else
          {
            sig=1;
            line[nr].bias=pow(bias, (1./((double)line[nr].div-1.)));
          }
          //printf(" bia:%f %f\n",bias);
          repLine(nr);
          goto new_bia;
        }
      }
    }
  }
      

  /* get the length between points */
  if(!i) /* straight line, no inner points */
  {
    min_cos_a=1;
    v_result(&line[nr].ip[i],&line[nr].ip[i+3], v0); 
    l=v_betrag(v0)/line[nr].div;
  }
  else
  {
    /* Calculate the length between points. It is sufficient to do it for the last sector */
    l=v_betrag(v0);
  }

  /* change the div */
  /* if the minimum cos_a in the line is greater than the target and the length is smaller as lmax reduce the division by factor of 2 */
  if((min_cos_a>gtol_cos_a)&&(line[nr].div>MIN_LINE_DIV)&&(l<lmax))
  {
    //printf("l:%f div:%d\n",l,line[nr].div);
    if(l*2<lmax) { line[nr].div/=2; bias=1.; line[nr].bias=1.; min_cos_a=MAX_FLOAT; max_cos_a=0.; goto new_div; }
    else line[nr].div=(double)line[nr].div*l/lmax;
  }

  /* check the division if the elements would be too small */
  if(l<=lmin)
  {
    line[nr].div=(double)line[nr].div*l/lmin;
  }

  if(line[nr].div<1) line[nr].div=MIN_LINE_DIV;
  if((MIN_LINE_DIV%2==0)&&(line[nr].div%2!=0)) line[nr].div+=1;

  /* final bias adjust */
  if(sig==-1)  // greater curvature at line end, smaller elem at end
  {
    line[nr].bias= 1./pow((bias), (1./((double)line[nr].div-1.)));
  }
  else if(sig==1)
  {
    line[nr].bias=pow(bias, (1./((double)line[nr].div-1.)));
  }
  else line[nr].bias=1;

  if(printFlag) printf("line:%s div:%d bias:%f gtol_cos_a:%lf min_cos_a:%lf lmax:%lf l:%lf\n", line[nr].name, line[nr].div, line[nr].bias, gtol_cos_a, min_cos_a, lmax*scale->w, l*scale->w);

  return(line[nr].div);
}




void repNurl(int nr )
{
  int i,j;

  /* calculation of the controll-point-array for drawing purposes */
  if( nurbl[nr].name != (char *)NULL )
  {
    if( (nurbl[nr].ctlarray = (GLfloat *)realloc( (GLfloat *)nurbl[nr].ctlarray, (nurbl[nr].u_npnt*nurbl[nr].u_stride)*sizeof(GLfloat) )) == NULL )
    { printf(" ERROR: realloc failure in repNurbl(), nurbl:%s can not be shaped\n\n", nurbl[nr].name);
      return; }

    /* calculate the position of the controll-array */
    j=0; for (i=0; i<nurbl[nr].u_npnt; i++ )
    {
      nurbl[nr].ctlarray[j++]=nurbl[nr].weight[i]*(GLfloat)point[nurbl[nr].ctlpnt[i]].px ;
      nurbl[nr].ctlarray[j++]=nurbl[nr].weight[i]*(GLfloat)point[nurbl[nr].ctlpnt[i]].py ;
      nurbl[nr].ctlarray[j++]=nurbl[nr].weight[i]*(GLfloat)point[nurbl[nr].ctlpnt[i]].pz ;
      nurbl[nr].ctlarray[j++]=nurbl[nr].weight[i] ;
    }
  }
}


void calcNurbsResolution( int nr)
{
  int i;
  double p0[3], p0p1[3], p1[3];
  double umin, umax,du,vmin,vmax,dv, lu, lv;
  Points tab_p[UV_STEPS+1];
  double dtab_u[UV_STEPS+1], dtab_v[UV_STEPS+1], dtab_05[UV_STEPS+1];

  sem_wait(&sem_g);
#if TEST
  printf("calcNurbsResolution-nurbs:%s \n", nurbs[nr].name);
#endif
  /* calculation of the u,v resolution based on real world scale */
  /* get the u and v range by looking into the knots */
  umin=nurbs[nr].uknt[0];
  umax=nurbs[nr].uknt[nurbs[nr].u_nknt-1];
  vmin=nurbs[nr].vknt[0];
  vmax=nurbs[nr].vknt[nurbs[nr].v_nknt-1];
  sem_post(&sem_g);
  du=(umax-umin)/(UV_STEPS-1);
  dv=(vmax-vmin)/(UV_STEPS-1);
  if( (printFlag) && ((umin<0.)||(vmin<0.)) )
  {
    printf(" WARNING: in NURBS:%s umin or vmin negative! They are set to 0. in evalNurbs()\n", nurbs[nr].name);
    printf(" umin:%lf umax:%lf du:%lf\n", umin,umax,du);
    printf(" vmin:%lf vmax:%lf dv:%lf\n", vmin,vmax,dv);
  }
  for(i=0; i<UV_STEPS; i++) { dtab_u[i]=umin+du*i; dtab_v[i]=vmin+dv*i; }

  for(i=0; i<UV_STEPS; i++) { dtab_05[i]=(vmax+vmin)*.5; }
  evalNurbs( nr, UV_STEPS, dtab_u, dtab_05, tab_p);
  lu=0.;
  for(i=1; i<UV_STEPS; i++)
  {
    p0[0]=tab_p[i-1].px;
    p0[1]=tab_p[i-1].py;
    p0[2]=tab_p[i-1].pz;
    p1[0]=tab_p[i].px;
    p1[1]=tab_p[i].py;
    p1[2]=tab_p[i].pz;
    v_result(p0,p1,p0p1);
    lu+=v_betrag(p0p1);
  }

  for(i=0; i<UV_STEPS; i++) { dtab_05[i]=(umax+umin)*.5; }
  evalNurbs( nr, UV_STEPS, dtab_05, dtab_v, tab_p);
  lv=0.;
  for(i=1; i<UV_STEPS; i++)
  {
    p0[0]=tab_p[i-1].px;
    p0[1]=tab_p[i-1].py;
    p0[2]=tab_p[i-1].pz;
    p1[0]=tab_p[i].px;
    p1[1]=tab_p[i].py;
    p1[2]=tab_p[i].pz;
    v_result(p0,p1,p0p1);
    lv+=v_betrag(p0p1);
  }

  /* average resolution */
  sem_wait(&sem_g);
  nurbs[nr].ures=lu/(umax-umin);
  nurbs[nr].vres=lv/(vmax-vmin);
  sem_post(&sem_g);
}



void repNurs(int nr )
{
  int i,j;

  if( nurbs[nr].name == (char *)NULL ) return;

  /* calculation of an average position for the name-string */
  nurbs[nr].tx=nurbs[nr].ty=nurbs[nr].tz=0.;
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    for (j=0; j<nurbs[nr].v_npnt; j++)
    {
      nurbs[nr].tx+=point[nurbs[nr].ctlpnt[i][j]].px ;
      nurbs[nr].ty+=point[nurbs[nr].ctlpnt[i][j]].py ;
      nurbs[nr].tz+=point[nurbs[nr].ctlpnt[i][j]].pz ;
    }
  }
  nurbs[nr].tx/=nurbs[nr].u_npnt*nurbs[nr].v_npnt;
  nurbs[nr].ty/=nurbs[nr].u_npnt*nurbs[nr].v_npnt;
  nurbs[nr].tz/=nurbs[nr].u_npnt*nurbs[nr].v_npnt;

  /* calculation of the controll-point-array for drawing purposes */
  if( nurbs[nr].name != (char *)NULL )
  {
    if( (nurbs[nr].ctlarray = (GLfloat *)realloc( (GLfloat *)nurbs[nr].ctlarray, (nurbs[nr].u_npnt*nurbs[nr].v_npnt*nurbs[nr].v_stride+5)*sizeof(GLfloat) )) == NULL )
    { printf(" ERROR: realloc failure in repNurbs(), nurbs:%s can not be shaped\n\n", nurbs[nr].name);
      return; }
     
    /* calculate the position of the control-array */
    for (i=0; i<nurbs[nr].u_npnt; i++)
    {
      for (j=0; j<nurbs[nr].v_npnt; j++)
      {
        nurbs[nr].ctlarray[i*(nurbs[nr].v_stride*nurbs[nr].v_npnt)+j*(nurbs[nr].v_stride)+0]=nurbs[nr].weight[i][j]*point[nurbs[nr].ctlpnt[i][j]].px;
        nurbs[nr].ctlarray[i*(nurbs[nr].v_stride*nurbs[nr].v_npnt)+j*(nurbs[nr].v_stride)+1]=nurbs[nr].weight[i][j]*point[nurbs[nr].ctlpnt[i][j]].py;
        nurbs[nr].ctlarray[i*(nurbs[nr].v_stride*nurbs[nr].v_npnt)+j*(nurbs[nr].v_stride)+2]=nurbs[nr].weight[i][j]*point[nurbs[nr].ctlpnt[i][j]].pz;
        nurbs[nr].ctlarray[i*(nurbs[nr].v_stride*nurbs[nr].v_npnt)+j*(nurbs[nr].v_stride)+3]=nurbs[nr].weight[i][j];
      }
    }
  }
}



/* delete the trimming data which where created in repSurf before */
void untrimNurs(int nr )
{
  int patch, curve;

  if( nurbs[nr].name == (char *)NULL ) return;

  for(patch=0; patch<nurbs[nr].patches; patch++)
  {
    for(curve=0; curve<nurbs[nr].nc[patch]; curve++)
    {
      free(nurbs[nr].uv[patch][curve]);
      free(nurbs[nr].xyz[patch][curve]);
    }
    free(nurbs[nr].np[patch]);
    free(nurbs[nr].uv[patch]);
    free(nurbs[nr].xyz[patch]);
  }
  if(!nurbs[nr].np)  free(nurbs[nr].np);
  if(!nurbs[nr].uv)  free(nurbs[nr].uv);
  if(!nurbs[nr].xyz) free(nurbs[nr].xyz);
  if(!nurbs[nr].nc)  free(nurbs[nr].nc);
  if(!nurbs[nr].vmax) free(nurbs[nr].vmax);
  if(!nurbs[nr].umax) free(nurbs[nr].umax);
  if(!nurbs[nr].vstep) free(nurbs[nr].vstep);
  if(!nurbs[nr].ustep) free(nurbs[nr].ustep);

  nurbs[nr].np=NULL;
  nurbs[nr].uv=NULL;
  nurbs[nr].xyz=NULL;

  nurbs[nr].nc=NULL;
  nurbs[nr].vmax=NULL;
  nurbs[nr].umax=NULL;
  nurbs[nr].vstep=NULL;
  nurbs[nr].ustep=NULL;
  
  nurbs[nr].patches=0;
}


/* return 0: node hidden, 1:node visible */
int plotNode(int nr)
{
  extern char drawMode;
  extern double *colNr;

  int j,hits;
  int node_zmin;
  GLint viewport[4];
  GLdouble mvmatrix[16], projmatrix[16];

  static GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
  GLuint size_selectBuf;
  static GLuint *selectBuf=NULL;
  double dum;

  dum=(double)anz->n+anz->e+anz->f+anz->g+anz->t+anzGeo->p+anzGeo->l+anzGeo->s+anzGeo->b+anzGeo->sh+anzGeo->nurl+anzGeo->nurs+1000;
  dum*=10;
  if(dum > MAX_INTEGER) size_selectBuf=MAX_INTEGER;
  else size_selectBuf=(GLuint)dum;
  do{
    if( ( selectBuf= (GLuint *)realloc((GLuint *)selectBuf, size_selectBuf * sizeof(GLuint))) == NULL )
    {
      printf ("WARNING: in Pick() is size_selectBuf: %d to large and is reduced\n", size_selectBuf);
      size_selectBuf/=2;
    }
    if(size_selectBuf<100)
    {
      errMsg("\n\n ERROR: realloc Failure in plotNode()\n\n") ;
      return(0);
    }
  }while(!selectBuf);

  glutSetWindow( w1);
  glGetIntegerv (GL_VIEWPORT, viewport);
  glSelectBuffer (size_selectBuf, selectBuf);

  /* get the window location of the node to specify the picking rectangle */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  moveModel();
  //glGetIntegerv (GL_VIEWPORT, viewport);
  glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
  if (gluProject( node[nr].nx, node[nr].ny, node[nr].nz, mvmatrix, projmatrix, viewport,  &wx, &wy, &wz)==GL_FALSE)
  {
    printf("WARNING: Malfunction in plotNode()\n");
    return(0);
  }

  /* get the min_z value of the node */
  glRenderMode (GL_SELECT); 
  glInitNames(); 
  glPushName (-1); 
  glLoadIdentity();
  gluPickMatrix( (GLdouble) wx, (GLdouble) wy, 1,1, viewport);
  moveModel();
  glColor3d ( 0.5,0.5,0.5 );
  glPointSize(10);
  glLoadName('n');
    glPushName(nr);
    glBegin ( GL_POINTS );
    glVertex3dv ( &node[nr].nx );
    glEnd();
    glPopName();
  glLoadName(-1);
  glFlush();
  hits = glRenderMode (GL_RENDER);
  if (hits<0)
  {
    errMsg("\nWARNING: Overflow occured!\n");
    free(selectBuf); selectBuf=NULL;
    return(0);
  }
  else
  {
    processHits( hits, selectBuf, "a   ", "a", wx, wy );
    node_zmin=pick_zmin;
  }

  /* get the min_z value of all other displayes entities */
  //glGetIntegerv (GL_VIEWPORT, viewport);
  glRenderMode (GL_SELECT); 
  glInitNames(); 
  glPushName (-1); 
  glLoadIdentity();
  gluPickMatrix( (GLdouble) wx, (GLdouble) wy, 1,1, viewport);
  moveModel();
  if (drawMode==4)
  {
    drawSets( PICK );
  }
  else
  {
    for (j=0; j<anzGeo->psets; j++ )
    {
      if(pset[j].type[0]=='e') drawElements_plot( set[pset[j].nr].anz_e, set[pset[j].nr].elem, node, colNr, e_enqire, 2, 0,1,PICK );
      if(pset[j].type[0]=='f') drawFaces_plot( set[pset[j].nr].anz_f, set[pset[j].nr].face, node, colNr, face, 2, 0, 1, PICK );
    }
  }

  glFlush();
  hits = glRenderMode (GL_RENDER);
  if (hits<0)
  {
    errMsg("\nWARNING: Overflow occured!\n");
    free(selectBuf); selectBuf=NULL;
    return(0);
  }
  else
  {
    processHits( hits, selectBuf, "a   ", "a", wx, wy );
    free(selectBuf); selectBuf=NULL;
    if((unsigned int)node_zmin<=(unsigned int)pick_zmin) return(1); else return(0);
  }
}



int adjustFeedBack( int npgn, GLfloat *pgn, double **ptr)
{
  int i, n=0, m=0, nv, nbuf;
  double v1[3],v2[3],vn[3];
  double vbuf[2][3];

  GLfloat token;
  double *pgn_new;

  /* create a new array for the polygons with its normals (guessed to be twice as large) */
  if( (pgn_new= (double *)malloc( 2*sizeof(double) )) == NULL )
  { printf(" ERROR: malloc failure in adjustFeedBack()\n\n"); exit(-1); }
  

  while((npgn-n))
  {
    token= pgn[n++];
    if(token==GL_POLYGON_TOKEN)
    {
      pgn_new[m++]=token;
      nv=pgn_new[m++]=pgn[n++];
      if(nv!=3) printf("GL_POLYGON_TOKEN with %d vertexes\n", nv);
      if( (pgn_new= (double *)realloc((double *)pgn_new, (m+nv*6+2)*sizeof(double) )) == NULL )
      { printf(" ERROR: malloc failure in adjustFeedBack()\n\n"); exit(-1); }
      nbuf=n;
      for( i=0; i<nv; i++)
      {
        pgn[n]=((double)pgn[n]*2./(double)width_w1-1.)*aspectRatio_w1; n++;  /* x */
        pgn[n]=(double)pgn[n]*2/(double)width_w1*aspectRatio_w1-1.; n++; /* y */
        pgn[n]-=0.5;  pgn[n++]*=-2.;   /* z */
      }

      /* calculate the normal and add to the start of the polygon */
      vbuf[0][0]=pgn[n-nv*3];
      vbuf[0][1]=pgn[n-nv*3+1];
      vbuf[0][2]=pgn[n-nv*3+2];
      vbuf[1][0]=pgn[3+n-nv*3];
      vbuf[1][1]=pgn[3+n-nv*3+1];
      vbuf[1][2]=pgn[3+n-nv*3+2];
      v_result(vbuf[0],vbuf[1], v1); 
      vbuf[1][0]=pgn[6+n-nv*3];
      vbuf[1][1]=pgn[6+n-nv*3+1];
      vbuf[1][2]=pgn[6+n-nv*3+2];
      v_result(vbuf[0],vbuf[1], v2);
      v_prod(v1,v2,vn);
      v_norm(vn,&pgn_new[m]); m+=3;
      for( i=0; i<nv; i++)
      {
        pgn_new[m++]=pgn[nbuf++];  /* x */
        pgn_new[m++]=pgn[nbuf++];  /* y */
        pgn_new[m++]=pgn[nbuf++];  /* z */
      }
   
    }
    else if(token==GL_POINT_TOKEN) { printf("ERROR: unsupported token %lf\n",token ); exit(-1); }
    else if(token==GL_LINE_TOKEN) { printf("ERROR: unsupported token %lf\n",token ); exit(-1); }
    else if(token==GL_LINE_RESET_TOKEN) { printf("ERROR: unsupported token %lf\n",token ); exit(-1); }
    else if(token==GL_PASS_THROUGH_TOKEN) { printf("ERROR: unsupported token %lf\n",token ); exit(-1); } 
    else { printf("ERROR: unknown token %lf\n",token ); exit(-1); }
  }

  *ptr=pgn_new;
  return(m);  
} 



/*
   it is not allowed to run fillBlendedSurf in parallel
   (reason: sets BLR,-Stmp would have to be used by all instances, one set per thread needed )
   it might be used in threads with sem_wait(&sem_rep), sem_post(&sem_rep) but eventually necessary
   surf[nr].sref=substituteSurf(nr) and the nurbs generation must have been done before!
*/
int fillBlendedSurf(int nr)
{
  int i,j,s;
  int setNr;
  int Stmp, sh_buf, setTmp;
  char buffer[MAX_LINE_LENGTH];

  int anz_p=0, anz_l=0, anz_s=0, newSetFlag=0, newSurfFlag=0;
  Points *ptmp=NULL;
  Lines *ltmp=NULL;
  Gsur *stmp=NULL;


  /* store surf in set */
  setNr=getSetNr(specialset->blr);
  if(setNr<0)
  {
    if(printFlag)
      printf(" WARNING in fillBlendedSurf(), set:%s does not exist and is initialized\n",specialset->blr);
    /* no threading environment, gen a set */
    setNr=pre_seta(specialset->blr, "i", 0);
    newSetFlag=1;   
  }
  sem_wait(&sem_g);
  seta(setNr,"s",nr);

  /* complete set */
  completeSet( specialset->blr, "do") ;
  sem_post(&sem_g);

  /* save the mesh data of the affected entities */
  if ((ptmp = (Points *)malloc((set[setNr].anz_p+1)*sizeof(Points)) ) == NULL )
  { printf("\n ERROR: malloc failure\n\n"); return(-1); }
  anz_p=set[setNr].anz_p;
  for(i=0; i<set[setNr].anz_p; i++)
  {
    if ((ptmp[i].nod = (int *)malloc((int)(point[set[setNr].pnt[i]].nn+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1);  }
    for(j=0; j<point[set[setNr].pnt[i]].nn; j++)
    {
      ptmp[i].nod[j]=point[set[setNr].pnt[i]].nod[j];
    }
    ptmp[i].nn=point[set[setNr].pnt[i]].nn;
    free(point[set[setNr].pnt[i]].nod);
    point[set[setNr].pnt[i]].nod=NULL; 
    point[set[setNr].pnt[i]].nn=0;
  }
  if ((ltmp = (Lines  *)malloc((set[setNr].anz_l+1)*sizeof(Lines )) ) == NULL )
  { printf("\n ERROR: malloc failure\n\n"); return(-1); }
  anz_l=set[setNr].anz_l;
  for(i=0; i<set[setNr].anz_l; i++)
  {
    if ((ltmp[i].nod = (int *)malloc((int)(line[set[setNr].line[i]].nn+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1); }
    for(j=0; j<line[set[setNr].line[i]].nn; j++)
    {
      ltmp[i].nod[j]=line[set[setNr].line[i]].nod[j];
    } 
    if ((ltmp[i].elem = (int *)malloc((int)(line[set[setNr].line[i]].ne+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1); }
    for(j=0; j<line[set[setNr].line[i]].ne; j++)
    {
      ltmp[i].elem[j]=line[set[setNr].line[i]].elem[j];
    } 
    ltmp[i].nn=line[set[setNr].line[i]].nn;
    ltmp[i].ne=line[set[setNr].line[i]].ne;
    ltmp[i].eattr=line[set[setNr].line[i]].eattr;
    free(line[set[setNr].line[i]].nod); free(line[set[setNr].line[i]].elem);
    line[set[setNr].line[i]].nod=NULL; line[set[setNr].line[i]].elem=NULL; 
    line[set[setNr].line[i]].nn=0; line[set[setNr].line[i]].ne=0;
  }
  if ((stmp = (Gsur   *)malloc((set[setNr].anz_s+1)*sizeof(Gsur  )) ) == NULL )
  { printf("\n ERROR: malloc failure\n\n"); return(-1); }
  anz_s=set[setNr].anz_s;
  for(i=0; i<set[setNr].anz_s; i++)
  {
    if ((stmp[i].nod = (int *)malloc((int)(surf[set[setNr].surf[i]].nn+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1); }
    for(j=0; j<surf[set[setNr].surf[i]].nn; j++)
    {
      stmp[i].nod[j]=surf[set[setNr].surf[i]].nod[j];
    } 
    if ((stmp[i].elem = (int *)malloc((int)(surf[set[setNr].surf[i]].ne+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n");  return(-1);}
    for(j=0; j<surf[set[setNr].surf[i]].ne; j++)
    {
      stmp[i].elem[j]=surf[set[setNr].surf[i]].elem[j];
    } 
    stmp[i].nn=surf[set[setNr].surf[i]].nn;
    stmp[i].ne=surf[set[setNr].surf[i]].ne;
    stmp[i].eattr=surf[set[setNr].surf[i]].eattr;
    //surf[set[setNr].surf[i]].eattr=0;
    free(surf[set[setNr].surf[i]].nod); free(surf[set[setNr].surf[i]].elem);
    surf[set[setNr].surf[i]].nod=NULL; surf[set[setNr].surf[i]].elem=NULL; 
    surf[set[setNr].surf[i]].nn=0; surf[set[setNr].surf[i]].ne=0;
  }


  /* mesh points and lines with the standard routines */
  meshPoints(setNr, 1) ;
  meshLines( setNr, 1);
  
  /* create the rendering poligons with an modified routine without mesh-improver */     
  // generate a substitute surf if necessary and no one is already asigned
  if(surf[nr].sref==-1)
  {
    //printf("Hallo surf:%s eattr:%d sh:%d\n",surf[nr].name, surf[nr].eattr, surf[nr].sh);
    // if structured mesh and not 4 lines:
    if((surf[nr].eattr>=0)&&(surf[nr].nl!=4))
    {
      //printf("structured mesh and not 4 lines, substituteSurf generates set zap\n");
      //printf("%d surf[nr].sref=%d\n",nr,surf[nr].sref);
      // somehow it is dangerous to save the return value from substituteSurf directly in surf
      // if the address of surf was changed in substituteSurf
      i=substituteSurf(nr);
      surf[nr].sref=i;
      newSurfFlag=1;
      meshSurf( 0, nr, 1, 1) ;
      delSurf( 1, &surf[nr].sref );
      surf[nr].sref=-1;
      goto surfrendered;
    }
    // if unstructured mesh:
    if(surf[nr].eattr==-1)
    {
      //printf("unstructured mesh \n");
      // if shape=BLEND try to generate a shape as a basis for the mesh 
      if(surf[nr].sh<0)
      {
        sh_buf=surf[nr].sh;
        if(surfToShape(nr)<0)
	{
	  //printf(" could not generate a shape, try structured mesh, substituteSurf generates set zap\n");
	  // could not generate a shape, try structured mesh
          if(surf[nr].nl!=4)
	  {
      // somehow it is dangerous to save the return value from substituteSurf directly in surf
      // if the address of surf was changed in substituteSurf
            i=substituteSurf(nr);
            surf[nr].sref=i;
            newSurfFlag=1;
	    if(surf[nr].sref>-1)
	    {
              meshSurf( 0, nr, 1, 1) ;
	      //printf(" substitute surf:%s eattr:%d sh:%d\n",surf[surf[nr].sref].name, surf[surf[nr].sref].eattr, surf[surf[nr].sref].sh);
              delSurf( 1, &surf[nr].sref );
	      surf[nr].sref=-1;
    	      goto surfrendered;
	    }
	  }
          surf[nr].eattr=0;
	  goto rendersurf;
	}
	// could generate a shape
      }

      //printf(" generate prelim nurbs\n");
      // if shape, generate prelim nurbs but only if it points not already to a nurbs
      if((shape[surf[nr].sh].type>-1)&&(shape[surf[nr].sh].type!=4))
      {
        Stmp=-1;
	Stmp= surfToNurs(nr);
    
        if(Stmp>-1)
        {
          sprintf(buffer,"-Stmp");
          delSet(buffer);
          setTmp=pre_seta( buffer, "S", nurbs[Stmp].name );
          if(setTmp<0)
          {
            printf(" ERROR: in meshSurf, temporary nurbs could not be added to set %s\n", buffer);
            return(-2);
          }
          completeSet( buffer, "do" );
    
          surf[nr].sh=shape_i( nurbs[Stmp].name, 4, Stmp, 0, 0, 0, 0, 0, 0);
          meshSurf( 0, nr, 1, 1) ;
          surf[nr].sh=sh_buf;
          zap(buffer);
          goto surfrendered;
	}
      }
    } 
  }
 rendersurf:;
  meshSurf( 0, nr, 1, 1) ;
 surfrendered:;

  /* delete the temporary entities which were created to substitute 3- and 5-sided surfs */
  /* warning, s is now redefined */
  if( newSurfFlag==1)
  {
    s=getSetNr(specialset->zap); 
    if(s>-1) 
    {
      for(i=0; i<set[s].anz_b; i++)
      {
	if(printFlag) printf (" delete body:%s \n",  body[set[s].body[i]].name );
  sem_wait(&sem_g);
	setr( 0, "b",set[s].body[i] );
  sem_post(&sem_g);
	body[set[s].body[i]].name = (char *)NULL ;
	body[set[s].body[i]].ns=0;
	free(body[set[s].body[i]].o);
	body[set[s].body[i]].o= NULL;
	free(body[set[s].body[i]].s);
	body[set[s].body[i]].s= NULL;
	body[set[s].body[i]].nn=0;
	free(body[set[s].body[i]].nod);
	body[set[s].body[i]].nod= NULL;
	body[set[s].body[i]].ne=0;
	free(body[set[s].body[i]].elem);
	body[set[s].body[i]].elem= NULL;      
	body[set[s].body[i]].etyp= 0;
      }
      for(i=0; i<set[s].anz_s; i++)
      {
	if(printFlag) printf (" delete surf:%s \n",  surf[set[s].surf[i]].name );
  sem_wait(&sem_g);
	setr( 0, "s",set[s].surf[i] );
  sem_post(&sem_g);
	surf[set[s].surf[i]].name = (char *)NULL ;
	surf[set[s].surf[i]].nl= 0;
	free(surf[set[s].surf[i]].typ);
	surf[set[s].surf[i]].typ= NULL;
	free(surf[set[s].surf[i]].o);
	surf[set[s].surf[i]].o= NULL;
	free(surf[set[s].surf[i]].l);
	surf[set[s].surf[i]].l= NULL;
	surf[set[s].surf[i]].nn= 0;
	free(surf[set[s].surf[i]].nod);
	surf[set[s].surf[i]].nod= NULL;
	surf[set[s].surf[i]].ne= 0;
	free(surf[set[s].surf[i]].elem);
	surf[set[s].surf[i]].elem= NULL;
	surf[set[s].surf[i]].etyp= 0;
      }
      for(i=0; i<set[s].anz_l; i++)
      {
	if(printFlag) printf (" delete line:%s \n",  line[set[s].line[i]].name );
  sem_wait(&sem_g);
	setr( 0, "l",set[s].line[i] );
  sem_post(&sem_g);
	line[set[s].line[i]].name = (char *)NULL ;
	line[set[s].line[i]].div = 0;
	if (line[set[s].line[i]].typ=='s')
	{
	  // delete the set
  sem_wait(&sem_g);
	  delSet(set[line[set[s].line[i]].trk].name);
  sem_post(&sem_g);
	}
	line[set[s].line[i]].typ=' ';
	line[set[s].line[i]].etyp=0;
	line[set[s].line[i]].p1=-1;
	line[set[s].line[i]].p2=-1;
	line[set[s].line[i]].trk=-1;
	line[set[s].line[i]].nip= 0;
	free(line[set[s].line[i]].ip);
	line[set[s].line[i]].ip= NULL;
	line[set[s].line[i]].nn= 0;
	free(line[set[s].line[i]].nod);
	line[set[s].line[i]].nod = NULL;
	line[set[s].line[i]].ne= 0;
	free(line[set[s].line[i]].elem);
	line[set[s].line[i]].elem = NULL;
      }
      for(i=0; i<set[s].anz_c; i++)
      {
	if(printFlag) printf (" delete lcmb:%s \n",  lcmb[set[s].lcmb[i]].name );
  sem_wait(&sem_g);
	setr( 0, "c",set[s].lcmb[i] );
  sem_post(&sem_g);
	lcmb[set[s].lcmb[i]].name = (char *)NULL;
	lcmb[set[s].lcmb[i]].nl=0;
	free(lcmb[set[s].lcmb[i]].o);
	lcmb[set[s].lcmb[i]].o= NULL;
	free(lcmb[set[s].lcmb[i]].l);
	lcmb[set[s].lcmb[i]].l= NULL;
	lcmb[set[s].lcmb[i]].p1=-1;
	lcmb[set[s].lcmb[i]].p2=-1;
      }
      for(i=0; i<set[s].anz_p; i++)
      {
	if(printFlag) printf (" delete pnt:%s \n",  point[set[s].pnt[i]].name );
  sem_wait(&sem_g);
	setr( 0, "p",set[s].pnt[i] );
  sem_post(&sem_g);
	point[set[s].pnt[i]].name = (char *)NULL ; 
	free(point[set[s].pnt[i]].nod);
	point[set[s].pnt[i]].nod=NULL; 
	point[set[s].pnt[i]].nn=0; 
      }
      // delete the set itself
  sem_wait(&sem_g);
      delSet(specialset->zap);
  sem_post(&sem_g);
    }
  }

  /* restore the mesh data of the affected entities */
  if(anz_p>set[setNr].anz_p) anz_p=set[setNr].anz_p;
  for(i=0; i<anz_p; i++)
  {
    point[set[setNr].pnt[i]].nn=ptmp[i].nn;
    if ((point[set[setNr].pnt[i]].nod = (int *)malloc((int)(point[set[setNr].pnt[i]].nn+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1);  }
    for(j=0; j<ptmp[i].nn; j++)
    {
      point[set[setNr].pnt[i]].nod[j]=ptmp[i].nod[j];
    }
    free(ptmp[i].nod);
    ptmp[i].nod=NULL; 
    ptmp[i].nn=0;
  }
  for(i=0; i<anz_l; i++)
  {
    line[set[setNr].line[i]].nn=ltmp[i].nn;
    line[set[setNr].line[i]].ne=ltmp[i].ne;
    if ((line[set[setNr].line[i]].nod = (int *)malloc((int)(line[set[setNr].line[i]].nn+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1); }
    if ((line[set[setNr].line[i]].elem = (int *)malloc((int)(line[set[setNr].line[i]].ne+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1); }
    for(j=0; j<ltmp[i].nn; j++)
    {
      line[set[setNr].line[i]].nod[j]=ltmp[i].nod[j];
    }
    for(j=0; j<ltmp[i].ne; j++)
    {
      line[set[setNr].line[i]].elem[j]=ltmp[i].elem[j];
    }
    free(ltmp[i].nod);
    ltmp[i].nod=NULL; 
    ltmp[i].nn=0;
    free(ltmp[i].elem);
    ltmp[i].elem=NULL; 
    ltmp[i].ne=0;
  }
  for(i=0; i<anz_s; i++)
  {
    surf[set[setNr].surf[i]].eattr=stmp[i].eattr;
    surf[set[setNr].surf[i]].nn=stmp[i].nn;
    surf[set[setNr].surf[i]].ne=stmp[i].ne;
    if (( surf[set[setNr].surf[i]].nod= (int *)malloc((int)(surf[set[setNr].surf[i]].nn+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n"); return(-1); }
    if (( surf[set[setNr].surf[i]].elem= (int *)malloc((int)(surf[set[setNr].surf[i]].ne+1)*sizeof(int)) ) == NULL )
    { printf(" ERROR: malloc failure\n\n");  return(-1);}
    for(j=0; j<stmp[i].nn; j++)
    {
      surf[set[setNr].surf[i]].nod[j]=stmp[i].nod[j];
    }
    for(j=0; j<stmp[i].ne; j++)
    {
      surf[set[setNr].surf[i]].elem[j]=stmp[i].elem[j];
    }
    free(stmp[i].nod);
    stmp[i].nod=NULL; 
    stmp[i].nn=0;
    free(stmp[i].elem);
    stmp[i].elem=NULL; 
    stmp[i].ne=0;
  }
  
  sem_wait(&sem_g);
  if( newSetFlag==1) { if(printFlag) printf("del %s\n",specialset->blr); delSet(specialset->blr); }  
  else
  {
    if(set[setNr].anz_elf)
     for(i=0; i<set[setNr].anz_elf; i++)
      if(set[setNr].elf[i].n) free(set[setNr].elf[i].v);  
    free(set[setNr].valu);
    free(set[setNr].node);
    free(set[setNr].elem);
    free(set[setNr].face);
    free(set[setNr].elf);
    free(set[setNr].pnt);
    free(set[setNr].line);
    free(set[setNr].lcmb);
    free(set[setNr].surf);
    free(set[setNr].body);
    free(set[setNr].nurl);
    free(set[setNr].nurs);
    free(set[setNr].set);
    free(set[setNr].shp);
    free(set[setNr].eparm);
    set[setNr].valu=(int *)NULL;
    set[setNr].node=(int *)NULL;
    set[setNr].elem=(int *)NULL;
    set[setNr].face=(int *)NULL;
    set[setNr].elf=(Elfaces *)NULL;
    set[setNr].pnt= (int *)NULL;
    set[setNr].line=(int *)NULL;
    set[setNr].lcmb=(int *)NULL;
    set[setNr].surf=(int *)NULL;
    set[setNr].body=(int *)NULL;
    set[setNr].nurl=(int *)NULL;
    set[setNr].nurs=(int *)NULL;
    set[setNr].set=(int *)NULL;
    set[setNr].shp=(int *)NULL;
    set[setNr].eparm=(char *)NULL;
    set[setNr].material = -1;
    set[setNr].flag = 'c';
    set[setNr].type = 0;
    set[setNr].anz_v = 0;
    set[setNr].anz_n = 0;
    set[setNr].anz_e = 0;
    set[setNr].anz_f = 0;
    set[setNr].anz_elf = 0;
    set[setNr].anz_p = 0;
    set[setNr].anz_l = 0;
    set[setNr].anz_c = 0;
    set[setNr].anz_s = 0;
    set[setNr].anz_b = 0;
    set[setNr].anz_nurl = 0;
    set[setNr].anz_nurs = 0;
    set[setNr].anz_se = 0;
    set[setNr].anz_sh = 0;
  }
  sem_post(&sem_g);
  return(1);
}


int _shapeToNurs(int s)
{
  int pbuf[4], lbuf[4], sbuf,Sbuf;
  double l_offs;
  double p1[3], p2[3], el[3], eh[3], pa1[3], pa2[3], ps1[3], ps2[3], nr[3];

    printf("shape:%s\n", shape[s].name);

    /* generate 3 points on the shape which cover the drawing space */
    v_result( &point[shape[s].p[0]].px, &point[shape[s].p[1]].px, p1);
    v_result( &point[shape[s].p[0]].px, &point[shape[s].p[2]].px, p2);

    /* generate 2 perpendicular vectors in this shape */
    v_prod( p1, p2, nr );
    v_prod( nr, p1, p2 );

    v_norm( p1, el );
    v_norm( p2, eh );

    l_offs=2.;
    v_scal(&l_offs, el, p1);
    v_add(&point[shape[s].p[0]].px, p1, pa1);
    v_scal(&l_offs, eh, p1);  
    v_add(&point[shape[s].p[0]].px, p1, ps1);
    l_offs=-2.;
    v_scal(&l_offs, el, p1);  
    v_add(&point[shape[s].p[0]].px, p1, pa2);
    v_scal(&l_offs, eh, p1);
    v_add(&point[shape[s].p[0]].px, p1, ps2);

    pbuf[0]=pnt( "-pa1", pa1[0], pa1[1], pa1[2], 0 );
    pbuf[1]=pnt( "-pa2", pa2[0], pa2[1], pa2[2], 0 );
    pbuf[2]=pnt( "-ps1", ps1[0], ps1[1], ps1[2], 0 );
    pbuf[3]=pnt( "-ps2", ps2[0], ps2[1], ps2[2], 0 );
    lbuf[0]=line_i( "-1l", pbuf[0], pbuf[2], 0, 1, 1, 0 );
    lbuf[1]=line_i( "-2l", pbuf[2], pbuf[1], 0, 1, 1, 0 );
    lbuf[2]=line_i( "-3l", pbuf[1], pbuf[3], 0, 1, 1, 0 );
    lbuf[3]=line_i( "-4l", pbuf[3], pbuf[0], 0, 1, 1, 0 );
    sbuf=surface_i( "-1s", '+', -1, (int)4, "++++", lbuf, "llll");
    Sbuf=createBlendedNurbs(sbuf);
    //repNurs(Sbuf );

    /* delete the temp.surf,line */
    delPnt( 4, pbuf );
    delLine( 4, lbuf );
    delSurf( 1, &sbuf );
    return(Sbuf);
}


int surfToShape(int s)
{
  /* check if the surf is plane and generate a shape if yes */

  int pbuf[4], sbuf, i,j,n,l,c;
  double p1[3], p2[3], el[3], eh[3], eq[3], nr[3];
  double ptrans[3];
  double maxlhq[3]={-MAX_FLOAT,-MAX_FLOAT,-MAX_FLOAT}, minlhq[3]={MAX_FLOAT,MAX_FLOAT,MAX_FLOAT};
  double ex[3], ey[3], ez[3], vx[3]={1.,0.,0.}, vy[3]={0.,1.,0.}, vz[3]={0.,0.,1.};
  double p_xyz[4][3], p_lhq[4][3];
  char buffer[MAX_LINE_LENGTH];

  /* generate 3 points on the surface */
  if(surf[s].typ[0]=='l') l=surf[s].l[0];
  else l=lcmb[surf[s].l[0]].l[0];
  sem_wait(&sem_g);
  v_result( &point[line[l].p1].px, &point[line[l].p2].px, p1);
  sem_post(&sem_g);
  for(i=0; i<surf[s].nl; i++)
  {
    if(surf[s].typ[i]=='l')
    {
      l=surf[s].l[i];
   sem_wait(&sem_g);
     v_result( &point[line[l].p1].px, &point[line[l].p2].px, p2);
  sem_post(&sem_g);
      v_prod( p1, p2, nr );
      if (v_betrag(nr)>0.) break;
    }
    else
    {
      for(j=0;j<lcmb[surf[s].l[i]].nl; j++)
      {
        l=lcmb[surf[s].l[i]].l[j];
  sem_wait(&sem_g);
        v_result( &point[line[l].p1].px, &point[line[l].p2].px, p2);
  sem_post(&sem_g);
        v_prod( p1, p2, nr );
        if (v_betrag(nr)>0.) break;
      }
      if (v_betrag(nr)>0.) break;
    }
  }
  if (v_betrag(nr)==0.) { printf("ERROR in surfToShape, could not determine 3 independent points. Talk with the programmer.\n"); exit(0); }
  v_prod( nr, p1, p2 );

  /* check if the surface is plane (if the extention in eq is 0. ) */
  /* new coordinate system */
  v_norm( p1, el );
  v_norm( p2, eh );
  v_norm( nr, eq );

  /* transform the xyz unit-vectors into the lhq system */
  ex[0]=v_sprod(vx,el);
  ex[1]=v_sprod(vx,eh);
  ex[2]=v_sprod(vx,eq);
  ey[0]=v_sprod(vy,el);
  ey[1]=v_sprod(vy,eh);
  ey[2]=v_sprod(vy,eq);
  ez[0]=v_sprod(vz,el);
  ez[1]=v_sprod(vz,eh);
  ez[2]=v_sprod(vz,eq);

  /* transform all points of the surface into lhq coordinates */
  for(i=0; i<surf[s].nl; i++)
  {
    if(surf[s].typ[i]=='l')
    {
      l=surf[s].l[i];
      for (n=0; n<line[l].nip; n+=3)
      {
        for(j=0; j<3; j++)
        {
          ptrans[j]=line[l].ip[n]*ex[j] + line[l].ip[n+1]*ey[j] + line[l].ip[n+2]*ez[j];
          if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
          if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
	  }
	}
    }
    else
    {
      for (c=0; c<lcmb[surf[s].l[i]].nl; c++)
	{
        l=lcmb[surf[s].l[i]].l[c];
        for (n=0; n<line[l].nip; n+=3)
        {
          for(j=0; j<3; j++)
          {
            ptrans[j]=line[l].ip[n]*ex[j] + line[l].ip[n+1]*ey[j] + line[l].ip[n+2]*ez[j];
            if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
            if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
	    }
	  }
      }
    }
  }
 
  /* check if the surf is plane: */
  if((maxlhq[2]-minlhq[2])>1.e-12) return(-1);

  //for(j=0; j<3; j++) printf("maxlhq:%lf minlhq:%lf\n", maxlhq[j],minlhq[j]);

  /* create points from maxlhq and minlhq which enclose the surface (p1 1st quadrant, pn: math+) */
  p_lhq[0][0]=maxlhq[0];
  p_lhq[0][1]=maxlhq[1];
  p_lhq[0][2]=maxlhq[2];
  p_lhq[1][0]=minlhq[0];
  p_lhq[1][1]=maxlhq[1];
  p_lhq[1][2]=maxlhq[2];
  p_lhq[2][0]=minlhq[0];
  p_lhq[2][1]=minlhq[1];
  p_lhq[2][2]=maxlhq[2];

  /* transform in xyz */
  for(i=0; i<3; i++) for(j=0; j<3; j++) p_xyz[i][j]=p_lhq[i][0]*el[j]+p_lhq[i][1]*eh[j]+p_lhq[i][2]*eq[j];
  //for(i=0; i<3; i++) for(j=0; j<3; j++) printf("p_xyz[%d]:%lf  %lf %lf %lf %lf %lf %lf \n", i, p_xyz[i][j], p_lhq[i][0],el[j],p_lhq[i][1],eh[j],p_lhq[i][2],eq[j]);

  sem_wait(&sem_g);
  for(i=0; i<3; i++)
  {
    getNewName( buffer, "p" );
    pbuf[i]=pnt( buffer , p_xyz[i][0], p_xyz[i][1], p_xyz[i][2], 0 );
  }
  getNewName( buffer, "sh" );
  sbuf=shape_i( buffer, 0, pbuf[0], pbuf[1], pbuf[2], 0, 0,0,0);
  sem_post(&sem_g);

  if(sbuf>-1) surf[s].sh=sbuf;
  printf("shape:%s %s %d\n", buffer, shape[sbuf].name, sbuf);
  return(sbuf);
}


int sphToNurs(int s, int flag)
{
  // 170*3 u++(v++)
  double controlPoint[]={
   0.000000, 0.500120, 0.000000,
    0.141180, 0.500130, -0.102600,
    0.282390, 0.431590, -0.205200,
    0.398670, 0.294550, -0.289690,
    0.465240, 0.105030, -0.338090,
    0.465240, -0.104770, -0.338120,
    0.398670, -0.294260, -0.289690,
    0.282370, -0.431340, -0.205220,
    0.141230, -0.499890, -0.102610,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.100910, 0.500130, -0.158010,
    0.201780, 0.431630, -0.316040,
    0.284880, 0.294540, -0.446230,
    0.332500, 0.105070, -0.520780,
    0.332500, -0.104740, -0.520810,
    0.284950, -0.294250, -0.446230,
    0.201840, -0.431300, -0.316120,
    0.100980, -0.499910, -0.158070,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.035710, 0.500110, -0.195360,
    0.071440, 0.431610, -0.390730,
    0.100850, 0.294590, -0.551750,
    0.117730, 0.105070, -0.643910,
    0.117720, -0.104730, -0.643940,
    0.100860, -0.294230, -0.551740,
    0.071430, -0.431340, -0.390810,
    0.035690, -0.499870, -0.195390,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.044820, 0.500130, -0.201680,
    -0.089670, 0.431600, -0.403310,
    -0.126590, 0.294570, -0.569450,
    -0.147730, 0.105060, -0.664560,
    -0.147740, -0.104740, -0.664580,
    -0.126580, -0.294240, -0.569440,
    -0.089680, -0.431330, -0.403330,
    -0.044820, -0.499910, -0.201690,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.123360, 0.500120, -0.169650,
    -0.246620, 0.431590, -0.339300,
    -0.348150, 0.294550, -0.479080,
    -0.406320, 0.105040, -0.559160,
    -0.406330, -0.104740, -0.559130,
    -0.348130, -0.294290, -0.479130,
    -0.246570, -0.431340, -0.339380,
    -0.123300, -0.499920, -0.169720,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.178010, 0.500120, -0.104910,
    -0.355920, 0.431580, -0.209820,
    -0.502530, 0.294560, -0.296220,
    -0.586450, 0.105050, -0.345720,
    -0.586450, -0.104760, -0.345740,
    -0.502520, -0.294280, -0.296270,
    -0.355930, -0.431340, -0.209840,
    -0.177950, -0.499900, -0.104920,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.196880, 0.500100, -0.026330,
    -0.393730, 0.431560, -0.052740,
    -0.555920, 0.294550, -0.074460,
    -0.648810, 0.105020, -0.086880,
    -0.648750, -0.104770, -0.086910,
    -0.555910, -0.294290, -0.074510,
    -0.393680, -0.431350, -0.052760,
    -0.196830, -0.499890, -0.026420,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.181500, 0.500110, 0.047200,
    -0.362980, 0.431600, 0.094370,
    -0.512410, 0.294510, 0.133200,
    -0.597990, 0.105020, 0.155420,
    -0.597990, -0.104790, 0.155400,
    -0.512400, -0.294330, 0.133150,
    -0.362930, -0.431330, 0.094290,
    -0.181450, -0.499910, 0.047190,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.141240, 0.500100, 0.102600,
    -0.282380, 0.431550, 0.205210,
    -0.398690, 0.294500, 0.289740,
    -0.465240, 0.104990, 0.338110,
    -0.465250, -0.104820, 0.338090,
    -0.398670, -0.294340, 0.289690,
    -0.282340, -0.431360, 0.205190,
    -0.141180, -0.499920, 0.102600,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.100900, 0.500080, 0.158090,
    -0.201790, 0.431550, 0.316170,
    -0.284900, 0.294510, 0.446280,
    -0.332450, 0.105000, 0.520860,
    -0.332450, -0.104810, 0.520830,
    -0.284900, -0.294310, 0.446280,
    -0.201750, -0.431350, 0.316150,
    -0.100930, -0.499900, 0.158060,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    -0.035700, 0.500110, 0.195440,
    -0.071390, 0.431600, 0.390860,
    -0.100820, 0.294510, 0.551850,
    -0.117680, 0.104990, 0.643990,
    -0.117680, -0.104820, 0.643960,
    -0.100810, -0.294330, 0.551800,
    -0.071390, -0.431350, 0.390780,
    -0.035650, -0.499910, 0.195430,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.044830, 0.500090, 0.201770,
    0.089730, 0.431590, 0.403380,
    0.126620, 0.294500, 0.569490,
    0.147780, 0.105000, 0.664630,
    0.147780, -0.104810, 0.664610,
    0.126630, -0.294310, 0.569500,
    0.089710, -0.431340, 0.403360,
    0.044860, -0.499880, 0.201730,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.123310, 0.500080, 0.169740,
    0.246610, 0.431590, 0.339430,
    0.348180, 0.294550, 0.479180,
    0.406380, 0.104990, 0.559180,
    0.406370, -0.104790, 0.559210,
    0.348190, -0.294290, 0.479130,
    0.246660, -0.431310, 0.339410,
    0.123340, -0.499890, 0.169700,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.178020, 0.500100, 0.104990,
    0.355970, 0.431620, 0.209950,
    0.502560, 0.294530, 0.296320,
    0.586490, 0.105010, 0.345790,
    0.586490, -0.104790, 0.345770,
    0.502580, -0.294300, 0.296270,
    0.355960, -0.431330, 0.209870,
    0.178060, -0.499880, 0.104910,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.196890, 0.500120, 0.026410,
    0.393730, 0.431610, 0.052810,
    0.555950, 0.294540, 0.074560,
    0.648800, 0.105020, 0.086960,
    0.648790, -0.104780, 0.086930,
    0.555970, -0.294300, 0.074510,
    0.393770, -0.431300, 0.052790,
    0.196940, -0.499900, 0.026410,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.181510, 0.500110, -0.047110,
    0.362980, 0.431590, -0.094240,
    0.512440, 0.294580, -0.133100,
    0.598030, 0.105040, -0.155350,
    0.598030, -0.104760, -0.155370,
    0.512460, -0.294250, -0.133150,
    0.363020, -0.431320, -0.094260,
    0.181550, -0.499870, -0.047200,
    -0.000010, -0.499910, -0.000010,
    0.000000, 0.500120, 0.000000,
    0.141180, 0.500130, -0.102600,
    0.282390, 0.431590, -0.205200,
    0.398670, 0.294550, -0.289690,
    0.465240, 0.105030, -0.338090,
    0.465240, -0.104770, -0.338120,
    0.398670, -0.294260, -0.289690,
    0.282370, -0.431340, -0.205220,
    0.141230, -0.499890, -0.102610,
    -0.000010, -0.499910, -0.000010
  };
  // 26U, 20V
  double controlNod[]={
      0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 2.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 0.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000
    , 1.000000 };

  int i,j,h,k,l,n,c;
  int nr=-1, pnr;
  char buffer[MAX_LINE_LENGTH];

  char name[MAX_LINE_LENGTH];
  int cp, pnts;
  double diameter, pbuf[3];
  double el[3], eh[3], eq[3];
  double p1[3], p2[3], nv[3], cg[3]={0,0,0};

  if(flag) h=surf[s].sh; else h=s;

  cp=shape[h].p[0];
  sem_wait(&sem_g);
  v_result( &point[shape[h].p[0]].px, &point[shape[h].p[1]].px, pbuf  );
  sem_post(&sem_g);
  diameter=v_betrag(pbuf)*2.;


  sem_wait(&sem_g);

  // use the shape pool to avoid conflicts with the master shape name
  nr= getNewName( name, "sh" );
  if ( nr == -1 )
    { printf(" ERROR: could not create new nurs\n"); return(-1); }

  // NURS ! DEFINE    8    9   17   10   26   20
  
  if ((nurbs = (Nurbs *)realloc( (Nurbs *)nurbs, (anzGeo->nurs+1)*sizeof(Nurbs)) ) == NULL )
  { printf("\n\nERROR: realloc failure in Nurs, nurbs:%s not installed\n\n", name); return(-1); }

  nr=anzGeo->nurs;
  hashNurs( sumAsci, name, nr );
  anzGeo->nurs++;
  i=strlen(name);
  if((nurbs[nr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
  { printf("ERROR: malloc failed\n\n" ); return(-1); }
  strcpy(nurbs[nr].name, name);

  /* save the original definition in a buffer */
  nurbs[nr].u_exp = 8;
  nurbs[nr].v_exp = 9;
  nurbs[nr].u_npnt= 17;
  nurbs[nr].v_npnt= 10;
  nurbs[nr].u_nknt= 26;
  nurbs[nr].v_nknt= 20;
  nurbs[nr].u_stride= 4* nurbs[nr].v_npnt;
  nurbs[nr].v_stride= 4;
  nurbs[nr].ctlarray=(GLfloat *)NULL;

  if ( (nurbs[nr].uknt = (GLfloat *)malloc( (nurbs[nr].u_nknt+1) * sizeof(GLfloat))) == NULL )
    printf("\n\n ERROR: realloc failed uknt\n\n");
  if ( (nurbs[nr].vknt = (GLfloat *)malloc( (nurbs[nr].v_nknt+1) * sizeof(GLfloat))) == NULL )
    printf("\n\n ERROR: realloc failed vknt\n\n");

  k=0;
  for(i=0; i<nurbs[nr].u_nknt; i++) { nurbs[nr].uknt[i]=controlNod[k++]; }
  for(i=0; i<nurbs[nr].v_nknt; i++) { nurbs[nr].vknt[i]=controlNod[k++]; }

  if ( (nurbs[nr].ctlpnt =
    (int **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(int *))) == NULL )
    printf("\n\n ERROR: malloc failed ctlpnt\n\n");
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    if ( (nurbs[nr].ctlpnt[i] =
      (int *)malloc(  (nurbs[nr].v_npnt+1) * sizeof( int ))) == NULL )
      printf("\n\n ERROR: malloc failed ctlpnt[i]\n\n");
  }

  if ( (nurbs[nr].weight =
    (GLfloat **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(GLfloat *))) == NULL )
    printf("\n\n ERROR: malloc failed weight\n\n");
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    if ( (nurbs[nr].weight[i] =
      (GLfloat *)malloc(  (nurbs[nr].v_npnt+1) * sizeof(GLfloat))) == NULL )
      printf("\n\n ERROR: malloc failed weight[i]\n\n");
  }

  k=0;
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    for (j=0; j<nurbs[nr].v_npnt; j++)
    {
      getNewName( buffer, "p" );
      pnr= pnt( buffer, (controlPoint[k]*diameter), (controlPoint[k+1]*diameter), (controlPoint[k+2]*diameter), 0);
      k+=3;
      nurbs[nr].ctlpnt[i][j] = pnr;
      nurbs[nr].weight[i][j]=1.;
    }
  }

  /* additional variables for the trimming and meshing */
  nurbs[nr].nurbsType=0;
  nurbs[nr].trimFlag=0;
  nurbs[nr].patches=0;

  nurbs[nr].uv=NULL;
  nurbs[nr].xyz=NULL;
  nurbs[nr].np=NULL;
  nurbs[nr].nc=NULL;
  nurbs[nr].umax=NULL;
  nurbs[nr].vmax=NULL;
  nurbs[nr].ustep=NULL;
  nurbs[nr].vstep=NULL;
  nurbs[nr].sum_ambiguousPnts=NULL;
  nurbs[nr].uvflipped=NULL;

  nurbs[nr].endFlag=1;       
  nurbs[nr].type=GL_MAP2_VERTEX_4;       
  
  nurbs[nr].Nurb = (GLUnurbsObj *)gluNewNurbsRenderer();

  repNurs(nr);

  for (i=0; i<anz->sets; i++)
  {
    if ( set[i].flag=='o') seta( i, "S", nr );
  }

  sem_post(&sem_g);

  /* align the nurs with the shape */
  /* transform all points of the surf into lhq coordinates */
  /* generate lhq coordinates */

  /* vector to the cg of the surface */
  if(flag)
  {
    pnts=0;
    for(i=0; i<surf[s].nl; i++)
    {
      if(surf[s].typ[i]=='l')
      {
        l=surf[s].l[i];
        pnts+=line[l].nip;
        for (n=0; n<line[l].nip; n+=3)
        {
          for(j=0; j<3; j++)
          {
            cg[j]+=line[l].ip[n+j];
  	}
        }
      }
      else
      {
        for (c=0; c<lcmb[surf[s].l[i]].nl; c++)
        {
          l=lcmb[surf[s].l[i]].l[c];
          pnts+=line[l].nip;
          for (n=0; n<line[l].nip; n+=3)
          {
            for(j=0; j<3; j++)
            {
              cg[j]+=line[l].ip[n+j];
  	  }
          }
        }
      }
    }
    pnts/=3;
    for(j=0; j<3; j++) cg[j]/=pnts;

    /* vector to u/2,v/2 */
    sem_wait(&sem_g);
    pnr=nurbs[nr].ctlpnt[6][5];
    v_result( &point[shape[h].p[0]].px, &point[pnr].px, p1);

    /* vector from origin to cg surface */
    v_result( &point[shape[h].p[0]].px, cg, p2);
    sem_post(&sem_g);
  }
  else
  {
    sem_wait(&sem_g);
    v_result( &point[shape[h].p[0]].px, &point[shape[h].p[1]].px, p1);
    v_result( &point[shape[h].p[0]].px, &point[shape[h].p[2]].px, p2);
    sem_post(&sem_g);
  }
  
#if TEST1
  v_norm( p1, p1 );
  for(j=0; j<3; j++)  printf("p:%s v sph xyz:%f\n",point[pnr].name,p1[j]);
  v_norm( p2, p2 );
  for(j=0; j<3; j++)  printf(" v surface xyz:%f\n",p2[j]);
#endif

  /* generate 2 perpendicular vectors in this shape */
  /* p2==eq must point in the direction of cg surf */
  v_prod( p1, p2, nv );
  v_prod( p2, nv, p1 );

  v_norm( nv, el );
  v_norm( p1, eh );
  v_norm( p2, eq );
#if TEST1
  for(j=0; j<3; j++)  printf("lhq xyz:%f %f %f\n",el[j],eh[j],eq[j]);
#endif

  /* transform in xyz */
  sem_wait(&sem_g);
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    for (j=0; j<nurbs[nr].v_npnt; j++)
    {
      pbuf[0]=point[nurbs[nr].ctlpnt[i][j]].px;
      pbuf[1]=point[nurbs[nr].ctlpnt[i][j]].py;
      pbuf[2]=point[nurbs[nr].ctlpnt[i][j]].pz;
      point[nurbs[nr].ctlpnt[i][j]].px= pbuf[0] *el[0] + pbuf[1] * eh[0] + pbuf[2] * eq[0] + point[cp].px;
      point[nurbs[nr].ctlpnt[i][j]].py= pbuf[0] *el[1] + pbuf[1] * eh[1] + pbuf[2] * eq[1] + point[cp].py;
      point[nurbs[nr].ctlpnt[i][j]].pz= pbuf[0] *el[2] + pbuf[1] * eh[2] + pbuf[2] * eq[2] + point[cp].pz;
    }
  }
#if TEST1
  v_result( &point[shape[h].p[0]].px, &point[pnr].px, p1);
  v_norm( p1, p1 );
  for(j=0; j<3; j++)  printf("p:%s v sph xyz:%f\n",point[pnr].name,p1[j]);
#endif
  repNurs(nr);
  sem_post(&sem_g);

  return(nr); // Nurbsindex
}


int torusToNurs(int s, int flag)
{
  // 81*4 u++(v++)
  double controlPoint[]={
0.000000, 3.000000, 0.000000, 1.000000  ,
-1.000000, 3.000000, 0.000000, 0.707107	,
-1.000000, 2.000000, 0.000000, 1.000000	,
-1.000000, 1.000000, 0.000000, 0.707107	,
0.000000, 1.000000, 0.000000, 1.000000 	,
1.000000, 1.000000, 0.000000, 0.707107 	,
1.000000, 2.000000, 0.000000, 1.000000 	,
1.000000, 3.000000, 0.000000, 0.707107 	,
0.000000, 3.000000, 0.000000, 1.000000 	,
0.000000, 3.000000, 3.000000, 0.707107 	,
-1.000000, 3.000000, 3.000000, 0.500000	,
-1.000000, 2.000000, 2.000000, 0.707107	,
-1.000000, 1.000000, 1.000000, 0.500000	,
0.000000, 1.000000, 1.000000, 0.707107 	,
1.000000, 1.000000, 1.000000, 0.500000 	,
1.000000, 2.000000, 2.000000, 0.707107 	,
1.000000, 3.000000, 3.000000, 0.500000 	,
0.000000, 3.000000, 3.000000, 0.707107 	,
0.000000, 0.000000, 3.000000, 1.000000 	,
-1.000000, 0.000000, 3.000000, 0.707107	,
-1.000000, 0.000000, 2.000000, 1.000000	,
-1.000000, 0.000000, 1.000000, 0.707107	,
0.000000, 0.000000, 1.000000, 1.000000 	,
1.000000, 0.000000, 1.000000, 0.707107 	,
1.000000, 0.000000, 2.000000, 1.000000 	,
1.000000, 0.000000, 3.000000, 0.707107 	,
0.000000, 0.000000, 3.000000, 1.000000 	,
0.000000, -3.000000, 3.000000, 0.707107	,
-1.000000, -3.000000, 3.000000, 0.500000,
-1.000000, -2.000000, 2.000000, 0.707107,
-1.000000, -1.000000, 1.000000, 0.500000,
0.000000, -1.000000, 1.000000, 0.707107 ,
1.000000, -1.000000, 1.000000, 0.500000 ,
1.000000, -2.000000, 2.000000, 0.707107 ,
1.000000, -3.000000, 3.000000, 0.500000 ,
0.000000, -3.000000, 3.000000, 0.707107 ,
0.000000, -3.000000, 0.000000, 1.000000 ,
-1.000000, -3.000000, 0.000000, 0.707107,
-1.000000, -2.000000, 0.000000, 1.000000,
-1.000000, -1.000000, 0.000000, 0.707107,
0.000000, -1.000000, 0.000000, 1.000000 ,
1.000000, -1.000000, 0.000000, 0.707107 ,
1.000000, -2.000000, 0.000000, 1.000000 ,
1.000000, -3.000000, 0.000000, 0.707107 ,
0.000000, -3.000000, 0.000000, 1.000000 ,
0.000000, -3.000000, -3.000000, 0.707107,
-1.000000, -3.000000, -3.000000, 0.500000,
-1.000000, -2.000000, -2.000000, 0.707107,
-1.000000, -1.000000, -1.000000, 0.500000,
0.000000, -1.000000, -1.000000, 0.707107, 
1.000000, -1.000000, -1.000000, 0.500000, 
1.000000, -2.000000, -2.000000, 0.707107, 
1.000000, -3.000000, -3.000000, 0.500000, 
0.000000, -3.000000, -3.000000, 0.707107, 
0.000000, 0.000000, -3.000000, 1.000000 , 
-1.000000, 0.000000, -3.000000, 0.707107, 
-1.000000, 0.000000, -2.000000, 1.000000, 
-1.000000, 0.000000, -1.000000, 0.707107, 
0.000000, 0.000000, -1.000000, 1.000000 , 
1.000000, 0.000000, -1.000000, 0.707107 , 
1.000000, 0.000000, -2.000000, 1.000000 , 
1.000000, 0.000000, -3.000000, 0.707107 , 
0.000000, 0.000000, -3.000000, 1.000000 , 
0.000000, 3.000000, -3.000000, 0.707107 , 
-1.000000, 3.000000, -3.000000, 0.500000, 
-1.000000, 2.000000, -2.000000, 0.707107, 
-1.000000, 1.000000, -1.000000, 0.500000, 
0.000000, 1.000000, -1.000000, 0.707107 , 
1.000000, 1.000000, -1.000000, 0.500000 , 
1.000000, 2.000000, -2.000000, 0.707107 , 
1.000000, 3.000000, -3.000000, 0.500000 , 
0.000000, 3.000000, -3.000000, 0.707107 , 
0.000000, 3.000000, 0.000000, 1.000000  , 
-1.000000, 3.000000, 0.000000, 0.707107 , 
-1.000000, 2.000000, 0.000000, 1.000000 , 
-1.000000, 1.000000, 0.000000, 0.707107 , 
0.000000, 1.000000, 0.000000, 1.000000  , 
1.000000, 1.000000, 0.000000, 0.707107	,
1.000000, 2.000000, 0.000000, 1.000000	,
1.000000, 3.000000, 0.000000, 0.707107	,
0.000000, 3.000000, 0.000000, 1.000000  ,
  };
  // 12U, 12V
  double controlNod[]={
    0.000000,
    0.000000,
    0.000000,
    0.250000,
    0.250000,
    0.500000,
    0.500000,
    0.750000,
    0.750000,
    1.000000,
    1.000000,
    1.000000,
    0.000000,
    0.000000,
    0.000000,
    0.250000,
    0.250000,
    0.500000,
    0.500000,
    0.750000,
    0.750000,
    1.000000,
    1.000000,
    1.000000,
  };


  int i,j,h,k,kbuf,l,n,c;
  int nr=-1, pnr;
  char buffer[MAX_LINE_LENGTH];

  char name[MAX_LINE_LENGTH];
  int cp,pnts, *vpnts;
  double scaleFac1,scaleFac2, pbuf[3];

  double master_radius1=2.;
  double master_radius2=1.;

  double el[3], eh[3], eq[3];
  double ex[3], ey[3], ez[3], vx[3]={1.,0.,0.}, vy[3]={0.,1.,0.}, vz[3]={0.,0.,1.};
  double p1[3], p2[3], nv[3], cg[3]={0,0,0}, cgs[3];
  double **cgt, vcgtcgs[3],vcgtpv0[3],vcgtpv1[3],fi;
  int nurbsbuf[2];
  
  if(flag) h=surf[s].sh; else h=s;

  cp=shape[h].p[0];
  sem_wait(&sem_g);
  v_result( &point[shape[h].p[0]].px, &point[shape[h].p[2]].px, pbuf  );
  sem_post(&sem_g);
  scaleFac1=v_betrag(pbuf) / master_radius1;
  sem_wait(&sem_g);
  v_result( &point[shape[h].p[2]].px, &point[shape[h].p[3]].px, pbuf  );
  sem_post(&sem_g);
  scaleFac2=v_betrag(pbuf) / master_radius2 /scaleFac1;

  //printf("r1:%f r2:%f\n", scaleFac1,scaleFac2);

  sem_wait(&sem_g);

  //strcpy(name,shape[h].name);
  //nr= getNewName( name, "S" );
  // use the shape pool to avoid conflicts with the master shape name
  nr= getNewName( name, "sh" );
  if ( nr == -1 )
    { printf(" ERROR: could not create new nurs\n"); return(-1); }


  if ((nurbs = (Nurbs *)realloc( (Nurbs *)nurbs, (anzGeo->nurs+1)*sizeof(Nurbs)) ) == NULL )
  { printf("\n\nERROR: realloc failure in Nurs, nurbs:%s not installed\n\n", name); return(-1); }

  nr=anzGeo->nurs;
  hashNurs( sumAsci, name, nr );
  anzGeo->nurs++;
  i=strlen(name);
  if((nurbs[nr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
  { printf("ERROR: malloc failed\n\n" ); return(-1); }

  strcpy(nurbs[nr].name, name);

  /* save the original definition in a buffer */
  nurbs[nr].u_exp = 2;
  nurbs[nr].v_exp = 2;
  nurbs[nr].u_npnt= 9;
  nurbs[nr].v_npnt= 9;
  nurbs[nr].u_nknt= 12;
  nurbs[nr].v_nknt= 12;
  nurbs[nr].u_stride= 4* nurbs[nr].v_npnt;
  nurbs[nr].v_stride= 4;
  nurbs[nr].ctlarray=(GLfloat *)NULL;

  if ( (nurbs[nr].uknt = (GLfloat *)malloc( (nurbs[nr].u_nknt+1) * sizeof(GLfloat))) == NULL )
    printf("\n\n ERROR: realloc failed uknt\n\n");
  if ( (nurbs[nr].vknt = (GLfloat *)malloc( (nurbs[nr].v_nknt+1) * sizeof(GLfloat))) == NULL )
    printf("\n\n ERROR: realloc failed vknt\n\n");

  k=0;
  for(i=0; i<nurbs[nr].u_nknt; i++) { nurbs[nr].uknt[i]=controlNod[k++]; }
  for(i=0; i<nurbs[nr].v_nknt; i++) { nurbs[nr].vknt[i]=controlNod[k++]; }

  if ( (nurbs[nr].ctlpnt =
    (int **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(int *))) == NULL )
    printf("\n\n ERROR: malloc failed ctlpnt\n\n");
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    if ( (nurbs[nr].ctlpnt[i] =
      (int *)malloc(  (nurbs[nr].v_npnt+1) * sizeof( int ))) == NULL )
      printf("\n\n ERROR: malloc failed ctlpnt[i]\n\n");
  }

  if ( (nurbs[nr].weight =
    (GLfloat **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(GLfloat *))) == NULL )
    printf("\n\n ERROR: malloc failed weight\n\n");
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    if ( (nurbs[nr].weight[i] =
      (GLfloat *)malloc(  (nurbs[nr].v_npnt+1) * sizeof(GLfloat))) == NULL )
      printf("\n\n ERROR: malloc failed weight[i]\n\n");
  }

  if ( (cgt= (double **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(double *))) == NULL )
    printf("\n\n ERROR: malloc failed\n\n");
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    if ( (cgt[i]= (double *)malloc(  (3) * sizeof(double))) == NULL )
    printf("\n\n ERROR: malloc failed\n\n");
  }

  nurbsbuf[0]=nurbs[nr].u_npnt;
  nurbsbuf[1]=nurbs[nr].v_npnt;
  sem_post(&sem_g);
  k=0;
  for (i=0; i<nurbsbuf[0]; i++)
  {
    /* scale the points of the tube relative to the local center */
    /* determine the center, u==0 has y=0 u==1 -> y+ */
    cgt[i][0]=0.;
    cgt[i][1]=0.;
    cgt[i][2]=0.;
    kbuf=k;
    for (j=1; j<nurbsbuf[1]; j++)
    {
      cgt[i][1]+=controlPoint[k+1];
      cgt[i][2]+=controlPoint[k+2];
      k+=4;
    }
    j--;
    cgt[i][1]/=j;
    cgt[i][2]/=j;
#if TEST1
    printf("cg torus section %f %f %f scal1 %f %f\n", cgt[i][0], cgt[i][1], cgt[i][2],scaleFac1,scaleFac2);
#endif
    k=kbuf;
    for (j=0; j<nurbsbuf[1]; j++)
    {
      /* all points of that u have to be scaled by 'radius' */
      controlPoint[k]=(controlPoint[k]-cgt[i][0])*scaleFac2 + cgt[i][0];
      controlPoint[k+1]=(controlPoint[k+1]-cgt[i][1])*scaleFac2 + cgt[i][1];
      controlPoint[k+2]=(controlPoint[k+2]-cgt[i][2])*scaleFac2 + cgt[i][2];

      /* scale the torus to the radius1 */
      controlPoint[k]=controlPoint[k]*scaleFac1;
      controlPoint[k+1]=controlPoint[k+1]*scaleFac1;
      controlPoint[k+2]=controlPoint[k+2]*scaleFac1;

      /* generate the nurbs controll point */
      // the last point is the first point
    sem_wait(&sem_g);
      if(j==nurbs[nr].v_npnt-1)
      {
        nurbs[nr].ctlpnt[i][j] = nurbs[nr].ctlpnt[i][0];
      }
      else
      {
        getNewName( buffer, "p" );
        pnr= pnt( buffer, (controlPoint[k]), (controlPoint[k+1]), (controlPoint[k+2]), 0);
        nurbs[nr].ctlpnt[i][j] = pnr;
      }
      nurbs[nr].weight[i][j]= controlPoint[k+3];
    sem_post(&sem_g);
      k+=4;
    }
    /* scale cgt */
    cgt[i][1]*=scaleFac1;
    cgt[i][2]*=scaleFac1;
  }

  /* additional variables for the trimming and meshing */
  sem_wait(&sem_g);
  nurbs[nr].nurbsType=0;
  nurbs[nr].trimFlag=0;
  nurbs[nr].patches=0;

  nurbs[nr].uv=NULL;
  nurbs[nr].xyz=NULL;
  nurbs[nr].np=NULL;
  nurbs[nr].nc=NULL;
  nurbs[nr].umax=NULL;
  nurbs[nr].vmax=NULL;
  nurbs[nr].ustep=NULL;
  nurbs[nr].vstep=NULL;
  nurbs[nr].sum_ambiguousPnts=NULL;
  nurbs[nr].uvflipped=NULL;

  nurbs[nr].endFlag=1;       
  nurbs[nr].type=GL_MAP2_VERTEX_4;       
  
  nurbs[nr].Nurb = (GLUnurbsObj *)gluNewNurbsRenderer();

  for (i=0; i<anz->sets; i++)
  {
    if ( set[i].flag=='o') seta( i, "S", nr );
  }

  /* align the nurs with the shape */

  /* transform all points of the nurs into lhq coordinates */
  /* generate lhq coordinates */
  /* axis vector */
  v_result( &point[shape[h].p[0]].px, &point[shape[h].p[1]].px, p1);
  sem_post(&sem_g);

  /* vector to the cg of the surface */
  if(flag)
  {
    pnts=0;
    for(i=0; i<surf[s].nl; i++)
    {
      if(surf[s].typ[i]=='l')
      {
        l=surf[s].l[i];
        pnts+=line[l].nip;
        for (n=0; n<line[l].nip; n+=3)
        {
          for(j=0; j<3; j++)
          {
            cg[j]+=line[l].ip[n+j];
  	}
        }
      }
      else
      {
        for (c=0; c<lcmb[surf[s].l[i]].nl; c++)
        {
          l=lcmb[surf[s].l[i]].l[c];
          pnts+=line[l].nip;
          for (n=0; n<line[l].nip; n+=3)
          {
            for(j=0; j<3; j++)
            {
              cg[j]+=line[l].ip[n+j];
  	  }
          }
        }
      }
    }
    pnts/=3;
    for(j=0; j<3; j++) cg[j]/=pnts;

    /* vector from torus origin to cg surface */
    sem_wait(&sem_g);
    v_result( &point[shape[h].p[0]].px, cg, p2);
    sem_post(&sem_g);
  }
  else
  {
    sem_wait(&sem_g);
    v_result( &point[shape[h].p[0]].px, &point[shape[h].p[2]].px, p2);
    sem_post(&sem_g);
  }
  
  /* generate 2 perpendicular vectors in this shape */
  /* orig
  v_prod( p1, p2, nv );
  v_prod( nv, p1, p2 );

  v_norm( p1, el );
  v_norm( p2, eh );
  v_norm( nv, eq );
  */
  /* p2==eq must point in the opposite direction of cg surf */
  v_prod( p1, p2, nv );
  v_prod( p1, nv, p2 );

  v_norm( p1, el );
  v_norm( nv, eh );
  v_norm( p2, eq );

  /* the tube of the torus must be rotated around its axis to keep the surf on one side */
  /* transform the xyz unit-vectors into the lhq system */
  ex[0]=v_sprod(vx,el);
  ex[1]=v_sprod(vx,eh);
  ex[2]=v_sprod(vx,eq);
  ey[0]=v_sprod(vy,el);
  ey[1]=v_sprod(vy,eh);
  ey[2]=v_sprod(vy,eq);
  ez[0]=v_sprod(vz,el);
  ez[1]=v_sprod(vz,eh);
  ez[2]=v_sprod(vz,eq);
  /* transform cg into lhq coordinates */
  sem_wait(&sem_g);
  for(j=0; j<3; j++)
  {
    cgs[j]=(cg[0]-point[shape[h].p[0]].px)*ex[j] + (cg[1]-point[shape[h].p[0]].py)*ey[j] + (cg[2]-point[shape[h].p[0]].pz)*ez[j];
#if TEST1
    printf(" cg surface lhq:%f xyz:%f\n",cgs[j],cg[j]);
#endif
  }
  sem_post(&sem_g);

  /* determine the rotation angle around the center of the tube */
  /* determine the vector from cgt[] to cgs */
  i=6;
  v_result(cgt[i],cgs,vcgtcgs);
  /* determine the vector from cgt[] to torus point at u==0 */

  sem_wait(&sem_g);
#if TEST1
  pnr=nurbs[nr].ctlpnt[i][2];
  printf(" nurbs_ctrp(%d,2):%s lhq:%f %f %f\n",i,point[pnr].name,point[pnr].px,point[pnr].py,point[pnr].pz);
  pnr=nurbs[nr].ctlpnt[i][1];
  printf(" nurbs_ctrp(%d,1):%s lhq:%f %f %f\n",i,point[pnr].name,point[pnr].px,point[pnr].py,point[pnr].pz);
#endif
  pnr=nurbs[nr].ctlpnt[i][0];
#if TEST1
  printf(" nurbs_ctrp(%d,0):%s lhq:%f %f %f\n",i,point[pnr].name,point[pnr].px,point[pnr].py,point[pnr].pz);
  for(j=0; j<3; j++) printf(" cgt lhq:%f\n",cgt[i][j]);
#endif
  v_result(cgt[i],&point[pnr].px,vcgtpv0);
  /* determine the rotational axis */
  pnr=nurbs[nr].ctlpnt[i][1];
  v_result(cgt[i],&point[pnr].px,vcgtpv1);
  sem_post(&sem_g);
  v_prod(vcgtpv0,vcgtpv1,nv);
  /* angle */
  fi=v_angle_ref(vcgtpv0,vcgtcgs,nv);
#if TEST1
  printf(" angle from surf to torus start deg:%f\n",fi*180./PI);
#endif
  /* since pv0 should be located opposite to cgs substract 180 deg */
  fi-=PI;
  /* only n*PI is allowed */
  if(fi<0.) fi+=2*PI;
#if TEST1
  printf(" deg:%f\n",fi*180./PI);
#endif

  /* rotate the points at all u */
  if(fi<PI*.25) k=1;
  else if(fi<PI*.75) k=3;
  else if(fi<PI*1.25) k=5;
  else if(fi<PI*1.75) k=7;
  else k=1;
  if(k>1)
  {
  sem_wait(&sem_g);
   if ( (vpnts = (int *)malloc(  (nurbs[nr].v_npnt+1) * sizeof(int))) == NULL )
     printf("\n\n ERROR: malloc failed vpnts\n\n");

   for (i=0; i<nurbs[nr].u_npnt; i++)
   {
    for (j=0; j<nurbs[nr].v_npnt-k; j++)
    {
      vpnts[j]=nurbs[nr].ctlpnt[i][j+k-1];
    }
    n=0;
    for (j=nurbs[nr].v_npnt-k; j<nurbs[nr].v_npnt-1; j++)
    {
      vpnts[j]=nurbs[nr].ctlpnt[i][n++];
    }
    vpnts[j]=vpnts[0];
    for (j=0; j<nurbs[nr].v_npnt; j++) nurbs[nr].ctlpnt[i][j]=vpnts[j];
    //for (j=0; j<nurbs[nr].v_npnt; j++) printf(" nurbs_ctrp(%d,%d):%s\n",i,j,point[nurbs[nr].ctlpnt[i][j]].name);
   }
  sem_post(&sem_g);
   free(vpnts);
  }

  /* transform the torus in xyz */
  sem_wait(&sem_g);
  for (i=0; i<nurbs[nr].u_npnt; i++)
  {
    for (j=0; j<nurbs[nr].v_npnt-1; j++)
    {
      pbuf[0]=point[nurbs[nr].ctlpnt[i][j]].px;
      pbuf[1]=point[nurbs[nr].ctlpnt[i][j]].py;
      pbuf[2]=point[nurbs[nr].ctlpnt[i][j]].pz;
      point[nurbs[nr].ctlpnt[i][j]].px= pbuf[0] *el[0] + pbuf[1] * eh[0] + pbuf[2] * eq[0] + point[cp].px;
      point[nurbs[nr].ctlpnt[i][j]].py= pbuf[0] *el[1] + pbuf[1] * eh[1] + pbuf[2] * eq[1] + point[cp].py;
      point[nurbs[nr].ctlpnt[i][j]].pz= pbuf[0] *el[2] + pbuf[1] * eh[2] + pbuf[2] * eq[2] + point[cp].pz;
    }
  }
  repNurs(nr);

  for (i=0; i<nurbs[nr].u_npnt; i++) free(cgt[i]);
  sem_post(&sem_g);
  free(cgt);

  return(nr); // Nurbsindex
}



int coneToNurs(int s, int flag)
{
  int pbuf[9][2], i, j, h, n, l, k, c, pnts, S;
  double p1[3], p2[3], cg[3]={0,0,0}, el[3], eh[3], eq[3], nr[3];
  double ptrans[3], dl,dr,qdrdl, dr1,dr2;
  //double maxlhq[3]={-MAX_FLOAT,-MAX_FLOAT,-MAX_FLOAT}, minlhq[3]={MAX_FLOAT,MAX_FLOAT,MAX_FLOAT};
  double maxlhq[3]={-2,-2,-2}, minlhq[3]={2,2,2};
  double ex[3], ey[3], ez[3], vx[3]={1.,0.,0.}, vy[3]={0.,1.,0.}, vz[3]={0.,0.,1.};
  double p_xyz[16][3], p_lhq[16][3];
  char buffer[MAX_LINE_LENGTH];
  double radius1, radius2;

  if(flag) h=surf[s].sh; else h=s;

  sem_wait(&sem_g);
  v_result( &point[shape[h].p[0]].px, &point[shape[h].p[2]].px, p1  );
  sem_post(&sem_g);
  radius1=v_betrag(p1);
  if(shape[h].type==2)
  {
  sem_wait(&sem_g);
    v_result( &point[shape[h].p[1]].px, &point[shape[h].p[3]].px, p2  );
  sem_post(&sem_g);
    radius2=v_betrag(p2);
  }
  else radius2=radius1;
  //printf("r1,2 %f %f\n",radius1,radius2);

  /* generate lhq coordinates */
  /* axis vector */
  sem_wait(&sem_g);
  v_result( &point[shape[h].p[0]].px, &point[shape[h].p[1]].px, p1);
  sem_post(&sem_g);

  /* vector to the cg of the surface */
  if(flag)
  {
    pnts=0;
    for(i=0; i<surf[s].nl; i++)
    {
      if(surf[s].typ[i]=='l')
      {
        l=surf[s].l[i];
        pnts+=line[l].nip;
        for (n=0; n<line[l].nip; n+=3)
        {
          for(j=0; j<3; j++)
          {
            cg[j]+=line[l].ip[n+j];
  	  }
        }
      }
      else
      {
        for (c=0; c<lcmb[surf[s].l[i]].nl; c++)
        {
          l=lcmb[surf[s].l[i]].l[c];
          pnts+=line[l].nip;
          for (n=0; n<line[l].nip; n+=3)
          {
            for(j=0; j<3; j++)
            {
              cg[j]+=line[l].ip[n+j];
  	  }
          }
        }
      }
    }
    pnts/=3;
    for(j=0; j<3; j++) cg[j]/=pnts;
    sem_wait(&sem_g);
    v_result( &point[shape[h].p[0]].px, cg, p2);
    sem_post(&sem_g);
  }
  else
  {
    sem_wait(&sem_g);
    v_result( &point[shape[h].p[0]].px, &point[shape[h].p[2]].px, p2);
    sem_post(&sem_g);
  }

  /* generate 2 perpendicular vectors in this shape */
  v_prod( p1, p2, nr );
  v_prod( nr, p1, p2 );

  v_norm( p1, el );
  v_norm( p2, eh );
  v_norm( nr, eq );

  /* transform the xyz unit-vectors into the lhq system */
  ex[0]=v_sprod(vx,el);
  ex[1]=v_sprod(vx,eh);
  ex[2]=v_sprod(vx,eq);
  ey[0]=v_sprod(vy,el);
  ey[1]=v_sprod(vy,eh);
  ey[2]=v_sprod(vy,eq);
  ez[0]=v_sprod(vz,el);
  ez[1]=v_sprod(vz,eh);
  ez[2]=v_sprod(vz,eq);

  /* transform all points of the surface into lhq coordinates */
  if(flag)
  {
    for(i=0; i<surf[s].nl; i++)
    {
      if(surf[s].typ[i]=='l')
      {
        l=surf[s].l[i];
        for (n=0; n<line[l].nip; n+=3)
        {
          for(j=0; j<3; j++)
          {
    sem_wait(&sem_g);
            ptrans[j]=(line[l].ip[n]-point[shape[h].p[0]].px)*ex[j] + (line[l].ip[n+1]-point[shape[h].p[0]].py)*ey[j] + (line[l].ip[n+2]-point[shape[h].p[0]].pz)*ez[j];
    sem_post(&sem_g);
            if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
            if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
          }
        }
      }
      else
      {
        for (c=0; c<lcmb[surf[s].l[i]].nl; c++)
        {
          l=lcmb[surf[s].l[i]].l[c];
          for (n=0; n<line[l].nip; n+=3)
          {
            for(j=0; j<3; j++)
            {
    sem_wait(&sem_g);
              ptrans[j]=(line[l].ip[n]-point[shape[h].p[0]].px)*ex[j] + (line[l].ip[n+1]-point[shape[h].p[0]].py)*ey[j] + (line[l].ip[n+2]-point[shape[h].p[0]].pz)*ez[j];
    sem_post(&sem_g);
              if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
              if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
  	    }
  	  }
        }
      }
    }
  }
  else
  {
    if(shape[h].type==2)
    {
      for(j=0; j<3; j++)
      {
      sem_wait(&sem_g);
        ptrans[j]=(point[shape[h].p[1]].px-point[shape[h].p[0]].px)*ex[j] + (point[shape[h].p[1]].py-point[shape[h].p[0]].py)*ey[j] + (point[shape[h].p[1]].pz-point[shape[h].p[0]].pz)*ez[j];
      sem_post(&sem_g);
        if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
        if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
      sem_wait(&sem_g);
        ptrans[j]=-2.;
      sem_post(&sem_g);
        if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
        if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
      }
    }
  }
  //for(j=0; j<3; j++) printf("maxlhq[%d]:%lf minlhq[%d]:%lf\n", j, maxlhq[j],j,minlhq[j]);
  
  /* generation of the nurbs */
  /* points */
  //for(i=0; i<8; i++) p_lhq[i][0]=(maxlhq[0]-minlhq[0])*-0.1;
  //for(i=8; i<16;i++) p_lhq[i][0]=(maxlhq[0]-minlhq[0])*1.1;
  
  /* The nurbs has to be adapted to the dimensions of the surf to make sure that a later trimming is possible */
  /* cone: scale also r1 and r2 accordingly */
  dl=v_betrag(p1);
  dr=radius2-radius1;
  qdrdl=dr/dl;
  dr2=maxlhq[0]*qdrdl;
  radius2=radius1+dr2;
  dr1=minlhq[0]*qdrdl;
  radius1+=dr1;
  //printf("r1,2 %f %f\n",radius1,radius2);

  for(i=0; i<8; i++) p_lhq[i][0]=minlhq[0];
  p_lhq[0][1]=-radius1;
  p_lhq[1][1]=-radius1;
  p_lhq[2][1]=0.     ;
  p_lhq[3][1]= radius1;
  p_lhq[4][1]= radius1;
  p_lhq[5][1]= radius1;
  p_lhq[6][1]=0.     ;
  p_lhq[7][1]=-radius1;
  p_lhq[0][2]=0.     ;
  p_lhq[1][2]=-radius1;
  p_lhq[2][2]=-radius1;
  p_lhq[3][2]=-radius1;
  p_lhq[4][2]=0.     ;
  p_lhq[5][2]= radius1;
  p_lhq[6][2]= radius1;
  p_lhq[7][2]= radius1;
  for(i=8; i<16;i++) p_lhq[i][0]=maxlhq[0];
  p_lhq[8][1]=-radius2;
  p_lhq[9][1]=-radius2;
  p_lhq[10][1]=0.     ;
  p_lhq[11][1]= radius2;
  p_lhq[12][1]= radius2;
  p_lhq[13][1]= radius2;
  p_lhq[14][1]=0.     ;
  p_lhq[15][1]=-radius2;
  p_lhq[8][2]=0.     ;
  p_lhq[9][2]=-radius2;
  p_lhq[10][2]=-radius2;
  p_lhq[11][2]=-radius2;
  p_lhq[12][2]=0.     ;
  p_lhq[13][2]= radius2;
  p_lhq[14][2]= radius2;
  p_lhq[15][2]= radius2;

  //for(i=0; i<16; i++) for(j=0; j<3; j++) printf("p_lhq[%d]:%lf\n", i, p_lhq[i][j] );

  /* transform in xyz */
  for(i=0; i<16; i++) for(j=0; j<3; j++) p_xyz[i][j]=p_lhq[i][0]*el[j]+p_lhq[i][1]*eh[j]+p_lhq[i][2]*eq[j];
  /* offset */
  sem_wait(&sem_g);
  for(i=0; i<16; i++) { p_xyz[i][0]+=point[shape[h].p[0]].px; p_xyz[i][1]+=point[shape[h].p[0]].py; p_xyz[i][2]+=point[shape[h].p[0]].pz; }
  sem_post(&sem_g);

  //for(i=0; i<16; i++) for(j=0; j<3; j++) printf("p_xyz[%d]:%lf\n", i, p_xyz[i][j] );

  k=0;
  for(j=0; j<2; j++) //v
  {
    for(i=0; i<8; i++)  //u
   {
    sprintf(buffer,"-pa%d", k+1);
    sem_wait(&sem_g);
    pbuf[i][j]=pnt( buffer , p_xyz[k][0], p_xyz[k][1], p_xyz[k][2], 0 );
    sem_post(&sem_g);
    k++;
   }
  }
  pbuf[8][0]=pbuf[0][0];
  pbuf[8][1]=pbuf[0][1];

  /* nurbs */

  sem_wait(&sem_g);

  // use the shape pool to avoid conflicts with the master shape name
  S= getNewName( buffer, "sh" );
  if ( S == -1 )
    { printf(" ERROR: could not create new nurs\n"); return(-1); }
  if ((nurbs = (Nurbs *)realloc( (Nurbs *)nurbs, (anzGeo->nurs+1)*sizeof(Nurbs)) ) == NULL )
  { printf("\n\nERROR: realloc failure in Nurs, nurbs:%s not installed\n\n", buffer); return(-1); }

  S=anzGeo->nurs;
  hashNurs( sumAsci, buffer, S );
  anzGeo->nurs++;
  if((nurbs[S].name= (char *)malloc((strlen(buffer)+1)*sizeof(char))) == NULL )
  { printf("ERROR: malloc failed\n\n" ); return(-1); }
  strcpy(nurbs[S].name, buffer);

  if(printFlag) printf("create NURBS:%s\n",nurbs[S].name);

  nurbs[S].u_exp = 2;
  nurbs[S].v_exp = 1;
  nurbs[S].u_npnt= 9;
  nurbs[S].v_npnt= 2;
  nurbs[S].u_nknt= 12;
  nurbs[S].v_nknt= 4;
  nurbs[S].u_stride= 4* nurbs[S].v_npnt;
  nurbs[S].v_stride= 4;

  if ( (nurbs[S].uknt = (GLfloat *)malloc( (nurbs[S].u_nknt+1) * sizeof(GLfloat))) == NULL )
    printf("\n\n ERROR: realloc failed uknt\n\n");
  if ( (nurbs[S].vknt = (GLfloat *)malloc( (nurbs[S].v_nknt+1) * sizeof(GLfloat))) == NULL )
    printf("\n\n ERROR: realloc failed vknt\n\n");
  j=0;
  for(i=0; i<3; i++) nurbs[S].uknt[j++]=0.;
  for(i=0; i<2; i++) nurbs[S].uknt[j++]=1.;
  for(i=0; i<2; i++) nurbs[S].uknt[j++]=2.;
  for(i=0; i<2; i++) nurbs[S].uknt[j++]=3.;
  for(i=0; i<3; i++) nurbs[S].uknt[j++]=4.;
  j=0;
  for(i=0; i<2; i++) nurbs[S].vknt[j++]=0.;
  for(i=0; i<2; i++) nurbs[S].vknt[j++]=1.;

  if ( (nurbs[S].ctlpnt =
    (int **)malloc(  (nurbs[S].u_npnt+1) * sizeof(int *))) == NULL )
    printf("\n\n ERROR: malloc failed ctlpnt\n\n");
  for (i=0; i<nurbs[S].u_npnt; i++)
  {
    if ( (nurbs[S].ctlpnt[i] =
      (int *)malloc(  (nurbs[S].v_npnt+1) * sizeof( int ))) == NULL )
      printf("\n\n ERROR: malloc failed ctlpnt[i]\n\n");
    for (j=0; j<nurbs[S].v_npnt; j++)
    {
      nurbs[S].ctlpnt[i][j] = pbuf[i][j];
    }
  }

  if ( (nurbs[S].weight = (float **)malloc(  (nurbs[S].u_npnt+1) * sizeof(float *))) == NULL )
    printf("\n\n ERROR: malloc failed weight\n\n");
  for (i=0; i<nurbs[S].u_npnt; i++)
  {
    if ( (nurbs[S].weight[i] = (float *)malloc(  (nurbs[S].v_npnt+1) * sizeof(float))) == NULL )
      printf("\n\n ERROR: malloc failed weight[i]\n\n");
  }

  k=0;
  for(j=0; j<2; j++) //v
  {
    for(i=0; i<8; i+=2) //u
    {
      nurbs[S].weight[i][j] = 1.;
      nurbs[S].weight[i+1][j] = .7071;
    }
  }
  nurbs[S].weight[8][0] = 1.;
  nurbs[S].weight[8][1] = 1.;

  /*
  for (i=0; i<nurbs[S].u_npnt; i++)
  {
    for (j=0; j<nurbs[S].v_npnt; j++) printf("%d %d %s %f\n", i,j, point[nurbs[S].ctlpnt[i][j]].name, nurbs[S].weight[i][j]);
  }
  for (i=0; i<nurbs[S].u_nknt; i++) printf("%d %f\n",i, nurbs[S].uknt[i]);
  for (i=0; i<nurbs[S].v_nknt; i++) printf("%d %f\n",i, nurbs[S].vknt[i]);
  */

  nurbs[S].ctlarray=(GLfloat *)NULL;
  nurbs[S].endFlag=1;       
  nurbs[S].type=GL_MAP2_VERTEX_4;       
  
  /* additional variables for the trimming */
  nurbs[S].trimFlag=0;
  nurbs[S].patches=0;
  nurbs[S].nc=NULL;
  nurbs[S].uv=NULL;
  nurbs[S].xyz=NULL;
  nurbs[S].np=NULL;
  nurbs[S].umax=NULL;
  nurbs[S].vmax=NULL;
  nurbs[S].ustep=NULL;
  nurbs[S].vstep=NULL;
  nurbs[S].Nurb = (GLUnurbsObj *)gluNewNurbsRenderer();
  nurbs[S].nurbsType=1;
  nurbs[S].uvflipped=NULL;
  nurbs[S].sum_ambiguousPnts=NULL;
  repNurs(S);
  sem_post(&sem_g);

  return(S);
}


int surfToNurs(int s)
{
  int pbuf[4], lbuf[4], sbuf,Sbuf, i,j,n,l,c;
  double p1[3], p2[3], el[3], eh[3], eq[3], nr[3];
  double ptrans[3], dlhq;
  double maxlhq[3]={-MAX_FLOAT,-MAX_FLOAT,-MAX_FLOAT}, minlhq[3]={MAX_FLOAT,MAX_FLOAT,MAX_FLOAT};
  double ex[3], ey[3], ez[3], vx[3]={1.,0.,0.}, vy[3]={0.,1.,0.}, vz[3]={0.,0.,1.};
  double p_xyz[4][3], p_lhq[4][3];
  char buffer[MAX_LINE_LENGTH];

  extern sem_t   sem_stn;

  //printf("surf:%s\n", surf[s].name);

  if((shape[surf[s].sh].type==1)||(shape[surf[s].sh].type==2)) return(coneToNurs(s,1));
  else if(shape[surf[s].sh].type==3) return(sphToNurs(s,1));
  else if(shape[surf[s].sh].type==4) return(-1);
  else if(shape[surf[s].sh].type==5) return(torusToNurs(s,1));

  /* default is pln */

  /* generate 3 points on the shape which cover the drawing space */
  sem_wait(&sem_g);
  v_result( &point[shape[surf[s].sh].p[0]].px, &point[shape[surf[s].sh].p[1]].px, p1);
  v_result( &point[shape[surf[s].sh].p[0]].px, &point[shape[surf[s].sh].p[2]].px, p2);
  sem_post(&sem_g);

  /* generate 2 perpendicular vectors in this shape */
  v_prod( p1, p2, nr );
  v_prod( nr, p1, p2 );

  v_norm( p1, el );
  v_norm( p2, eh );
  v_norm( nr, eq );

  /* transform the xyz unit-vectors into the lhq system */
  ex[0]=v_sprod(vx,el);
  ex[1]=v_sprod(vx,eh);
  ex[2]=v_sprod(vx,eq);
  ey[0]=v_sprod(vy,el);
  ey[1]=v_sprod(vy,eh);
  ey[2]=v_sprod(vy,eq);
  ez[0]=v_sprod(vz,el);
  ez[1]=v_sprod(vz,eh);
  ez[2]=v_sprod(vz,eq);

  /* transform all points of the surface into lhq coordinates */
  for(i=0; i<surf[s].nl; i++)
  {
    if(surf[s].typ[i]=='l')
    {
      l=surf[s].l[i];
      for (n=0; n<line[l].nip; n+=3)
      {
        for(j=0; j<3; j++)
        {
          ptrans[j]=line[l].ip[n]*ex[j] + line[l].ip[n+1]*ey[j] + line[l].ip[n+2]*ez[j];
          if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
          if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
	  }
	}
    }
    else
    {
      for (c=0; c<lcmb[surf[s].l[i]].nl; c++)
	{
        l=lcmb[surf[s].l[i]].l[c];
        for (n=0; n<line[l].nip; n+=3)
        {
          for(j=0; j<3; j++)
          {
            ptrans[j]=line[l].ip[n]*ex[j] + line[l].ip[n+1]*ey[j] + line[l].ip[n+2]*ez[j];
            if(ptrans[j]>maxlhq[j]) maxlhq[j]=ptrans[j];
            if(ptrans[j]<minlhq[j]) minlhq[j]=ptrans[j];
	    }
	  }
      }
    }
  }
  //for(j=0; j<3; j++) printf("maxlhq[%d]:%lf minlhq[%d]:%lf\n", j, maxlhq[j],j,minlhq[j]);

  /* The nurbs has to be extended a bit to make sure that a later trimming is possible */
  for(j=0; j<3; j++)
  {
    dlhq=(maxlhq[j]-minlhq[j])/2.*0.5; 
    maxlhq[j]+=dlhq;
    minlhq[j]-=dlhq;
  }
  //for(j=0; j<3; j++) printf("maxlhq[%d]:%lf minlhq[%d]:%lf\n", j, maxlhq[j],j,minlhq[j]);

  /* create points from maxlhq and minlhq which enclose the surface (p1 1st quadrant, pn: math+) */
  p_lhq[0][0]=maxlhq[0];
  p_lhq[0][1]=maxlhq[1];
  p_lhq[0][2]=maxlhq[2];
  p_lhq[1][0]=minlhq[0];
  p_lhq[1][1]=maxlhq[1];
  p_lhq[1][2]=maxlhq[2];
  p_lhq[2][0]=minlhq[0];
  p_lhq[2][1]=minlhq[1];
  p_lhq[2][2]=maxlhq[2];
  p_lhq[3][0]=maxlhq[0];
  p_lhq[3][1]=minlhq[1];
  p_lhq[3][2]=maxlhq[2];

  /* transform in xyz */
  for(i=0; i<4; i++) for(j=0; j<3; j++) p_xyz[i][j]=p_lhq[i][0]*el[j]+p_lhq[i][1]*eh[j]+p_lhq[i][2]*eq[j];
  //for(i=0; i<4; i++) for(j=0; j<3; j++) printf("p_xyz[%d]:%lf\n", i, p_xyz[i][j] );

  // only one thread per time allowed here (common names):
  if(sem_wait(&sem_stn)) printf("Error in:sem_wait\n");
  for(i=0; i<4; i++)
  {
    sprintf(buffer,"-pa%d", i+1);
    sem_wait(&sem_g);
    pbuf[i]=pnt( buffer , p_xyz[i][0], p_xyz[i][1], p_xyz[i][2], 0 );
    sem_post(&sem_g);
  }
  for(i=0; i<3; i++)
  {
    sprintf(buffer,"-l%d", i+1);
    // no new allocated line is allowed (threading), make sure a deleted line is available
    lbuf[i]=line_i( buffer, pbuf[i], pbuf[i+1], 0, 1, 1, 0 );
  }
  sprintf(buffer,"-l%d", i+1);
  // no new allocated line is allowed (threading), make sure a deleted line is available
  lbuf[i]=line_i( buffer, pbuf[i], pbuf[0], 0, 1, 1, 0 );

  // no new allocated surf is allowed (threading), make sure a deleted surf is available
  //printf("hallo surf2Nurs surf:%s ptr:%x\n", surf[s].name, surf);
  sbuf=surface_i( "-1s", '+', -1, (int)4, "++++", lbuf, "llll");
  //printf("hallo surf2Nurs surf:%s ptr:%x\n", surf[s].name, surf);

  Sbuf=createBlendedNurbs(sbuf);

  // was deactivated??
  repNurs(Sbuf );

  /* delete the temp.surf,line */
  delPnt( 4, pbuf );
  delLine( 4, lbuf );
  delSurf( 1, &sbuf );
  if(sem_post(&sem_stn)) printf("Error in:sem_post\n");

  return(Sbuf);
}


void descalAll(void)
{
  descalNodes ( anz->n, node, scale );
  descalPoints( anzGeo->p, point, scale);
  descalSurfs( anzGeo->s, surf, scale);
  scale->x=0.;
  scale->y=0.;
  scale->z=0.;
  scale->w=1.;
}


/*  store the polygons for illuminated rendering */
/* Warning: changes *scale */
int repShape(int setNr )
{
  int n=0, nr,j;
  double pa1[3], pa2[3], ps1[3], ps2[3];
  double l_offs;
  double p1[3], p2[3], pb[3];
  double el[3], eh[3];

  if(set[setNr].anz_sh==0) return(0);
 
  /* the drawing space has to be scaled to 2*2 */    
  //descalShapes( anzGeo->sh, shape, scale); TBD
  descalAll();
  getScaleValues( 0, set, point, node, scale);
  scalNodes ( anz->n, node, scale );
  scalPoints( anzGeo->p, point, scale);
  scalSurfs( anzGeo->s, surf, scale);


  for (j=0; j<set[setNr].anz_sh; j++)
  {
    nr=set[setNr].shp[j];

    if(printFlag) printf("shape:%s type:%d\n", shape[nr].name, shape[nr].type);

    if( shape[nr].type == 0)
    { 
      /* generate 3 points on the shape which cover the drawing space */
      v_result( &point[shape[nr].p[0]].px, &point[shape[nr].p[1]].px, p1);
      v_result( &point[shape[nr].p[0]].px, &point[shape[nr].p[2]].px, p2);
    
      /* generate 2 perpendicular vectors in this shape */
      v_prod( p1, p2, pb );
      v_prod( pb, p1, p2 );
    
      v_norm( p1, el );
      v_norm( p2, eh );
      l_offs=2.;
      v_scal(&l_offs, el, p1);
      v_add(&point[shape[nr].p[0]].px, p1, pa1);
      v_scal(&l_offs, eh, p1);  
      v_add(&point[shape[nr].p[0]].px, p1, ps1);
      l_offs=-2.;
      v_scal(&l_offs, el, p1);  
      v_add(&point[shape[nr].p[0]].px, p1, pa2);
      v_scal(&l_offs, eh, p1);
      v_add(&point[shape[nr].p[0]].px, p1, ps2);
    
      //printf("ps2: %f %f %f\n", ps2[0], ps2[1], ps2[2]);
    
      /* alloc a new tri */
      shape[nr].npgn=36;
      if((shape[nr].pgn=(GLdouble *)realloc((GLdouble *)shape[nr].pgn, shape[nr].npgn*sizeof(GLdouble)) )==NULL)
      {
        errMsg("\nERROR: realloc failed in repShape() \n\n");
      }
      n=0;
      shape[nr].pgn[n++]=ps2[0];
      shape[nr].pgn[n++]=ps2[1];
      shape[nr].pgn[n++]=ps2[2];
      shape[nr].pgn[n++]=pa1[0];
      shape[nr].pgn[n++]=pa1[1];
      shape[nr].pgn[n++]=pa1[2];
      shape[nr].pgn[n++]=point[shape[nr].p[0]].px;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].py;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].pz;
      shape[nr].pgn[n++]=pa1[0];
      shape[nr].pgn[n++]=pa1[1];
      shape[nr].pgn[n++]=pa1[2];
      shape[nr].pgn[n++]=ps1[0];
      shape[nr].pgn[n++]=ps1[1];
      shape[nr].pgn[n++]=ps1[2];
      shape[nr].pgn[n++]=point[shape[nr].p[0]].px;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].py;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].pz;
      shape[nr].pgn[n++]=ps1[0];
      shape[nr].pgn[n++]=ps1[1];
      shape[nr].pgn[n++]=ps1[2];
      shape[nr].pgn[n++]=pa2[0];
      shape[nr].pgn[n++]=pa2[1];
      shape[nr].pgn[n++]=pa2[2];
      shape[nr].pgn[n++]=point[shape[nr].p[0]].px;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].py;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].pz;
      shape[nr].pgn[n++]=pa2[0];
      shape[nr].pgn[n++]=pa2[1];
      shape[nr].pgn[n++]=pa2[2];
      shape[nr].pgn[n++]=ps2[0];
      shape[nr].pgn[n++]=ps2[1];
      shape[nr].pgn[n++]=ps2[2];
      shape[nr].pgn[n++]=point[shape[nr].p[0]].px;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].py;
      shape[nr].pgn[n++]=point[shape[nr].p[0]].pz;
    }
    else
    {
      if(printFlag) printf(" WARNING: shape:%s type:%d, surface rendering for this type is not implemented.\n", shape[nr].name, shape[nr].type );
      return(0);
    }
  }
  return(1);
}



int calcTrimLoops(int nurbsnr, int nr)
{
  int i,j,l,cl,nip,flag;
  int n;    /* running number through all inner-points ip in the line-def */  
  int k=0;  /* running number through all corners in the surf-def */
  int p;    /* running number through all points of each closed curve (trimming-loops) of the surf */
  int cp;    /* running number through all points of each closed curve (trimming-loops) of the surf */
  double *lcurve=NULL; /* tracks the length of all closed curves, the biggest is the outer loop. */
  double *clmax=NULL;  /* tracks the loop-direction (ccw or cw */
  double *nu=NULL, *nv=NULL;  /* sum of the u and v coordinates for all curves, averaged */  
  GLdouble *knt=NULL;           /* knot-buffer to invert nurbs-curves if necessary */
  double p0[3], p0p1[3], p0p2[3], p1[3], p2[3], p1p2[3], lp1p2, lmax=0.;
  double vn[3];
  int c_outer=0, patch;
  int nclp;
  double *xyz=NULL;

  int firstl, lastl;
  int *linbuf=NULL;
  char *oribuf=NULL;

  double tol_ambig;
  int nurbsbuf[2];


  /* redefine the nurbs (deactivated: ->ERROR: forbidden if a degree reduction was performed, the control-points were not updated!)*/
  //repNurs(nurbsnr);
  sem_wait(&sem_g);

  patch=surf[nr].patch=nurbs[nurbsnr].patches;
  nurbs[nurbsnr].patches++;
  printf("surf:%s NURBS:%s patch%d\n",surf[nr].name, nurbs[nurbsnr].name, patch);

  if( (nurbs[nurbsnr].umax= (double *)realloc( (double *)nurbs[nurbsnr].umax, (nurbs[nurbsnr].patches)*sizeof(double) )) == NULL )
  { printf(" ERROR: realloc failure0, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].vmax= (double *)realloc( (double *)nurbs[nurbsnr].vmax, (nurbs[nurbsnr].patches)*sizeof(double) )) == NULL )
  { printf(" ERROR: realloc failure1, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].ustep= (GLfloat *)realloc( (GLfloat *)nurbs[nurbsnr].ustep, (nurbs[nurbsnr].patches)*sizeof(GLfloat) )) == NULL )
  { printf(" ERROR: realloc failure, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].vstep= (GLfloat *)realloc( (GLfloat *)nurbs[nurbsnr].vstep, (nurbs[nurbsnr].patches)*sizeof(GLfloat) )) == NULL )
  { printf(" ERROR: realloc failure, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].nc= (int *)realloc( (int *)nurbs[nurbsnr].nc, (nurbs[nurbsnr].patches)*sizeof(int) )) == NULL )
  { printf(" ERROR: realloc failure2, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].np= (int **)realloc( (int **)nurbs[nurbsnr].np, (nurbs[nurbsnr].patches)*sizeof(int *) )) == NULL )
  { printf(" ERROR: realloc failure3, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].np[patch]= (int *)malloc( (surf[nr].nc)*sizeof(int) )) == NULL )
  { printf(" ERROR: realloc failure4, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }

  if( (nurbs[nurbsnr].uv = (GLfloat ***)realloc( (GLfloat ***)nurbs[nurbsnr].uv, (nurbs[nurbsnr].patches)*sizeof(GLfloat **) )) == NULL )
  { printf(" ERROR: realloc failure8, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].uv[patch] = (GLfloat **)malloc( (surf[nr].nc+1)*sizeof(GLfloat *) )) == NULL )
  { printf(" ERROR: realloc failure9, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].xyz = (double ***)realloc( (double ***)nurbs[nurbsnr].xyz, (nurbs[nurbsnr].patches)*sizeof(double **) )) == NULL )
  { printf(" ERROR: realloc failure10, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].xyz[patch] = (double **)malloc( (surf[nr].nc+1)*sizeof(double *) )) == NULL )
  { printf(" ERROR: realloc failure11, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }

  if( (nurbs[nurbsnr].uvflipped = (int **)realloc( (int **)nurbs[nurbsnr].uvflipped, (nurbs[nurbsnr].patches)*sizeof(int *) )) == NULL )
  { printf(" ERROR: realloc failure10, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].uvflipped[patch] = (int *)calloc( (surf[nr].nc+1),sizeof(int) )) == NULL )
  { printf(" ERROR: realloc failure11, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].sum_ambiguousPnts = (int **)realloc( (int **)nurbs[nurbsnr].sum_ambiguousPnts, (nurbs[nurbsnr].patches)*sizeof(int *) )) == NULL )
  { printf(" ERROR: realloc failure10, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nurbs[nurbsnr].sum_ambiguousPnts[patch] = (int *)calloc( (surf[nr].nc+1),sizeof(int) )) == NULL )
  { printf(" ERROR: realloc failure11, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }

  if( (lcurve= (double *)realloc( (double *)lcurve, (surf[nr].nc+1)*sizeof(double) )) == NULL )
  { printf(" ERROR: realloc failure5, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (clmax= (double *)realloc( (double *)clmax, (surf[nr].nc+1)*sizeof(double) )) == NULL )
  { printf(" ERROR: realloc failure5, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nu= (double *)realloc( (double *)nu, (surf[nr].nc+1)*sizeof(double) )) == NULL )
  { printf(" ERROR: realloc failure6, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  if( (nv= (double *)realloc( (double *)nv, (surf[nr].nc+1)*sizeof(double) )) == NULL )
  { printf(" ERROR: realloc failure7, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
    return(-1); }
  for (i=0; i<surf[nr].nc; i++) lcurve[i]=nu[i]=nv[i]=0.;

 newTrimming:;

  k=0;
  for (i=0; i<surf[nr].nc; i++)
  {
    nip=0;
    for(j=0; j<surf[nr].c[i]; j++)
    {
      if(surf[nr].typ[k]=='l')
      {
        l=surf[nr].l[k];
        nip+=line[l].nip;
      }
      else
      {
        nclp=0;
        for(cl=0; cl<lcmb[surf[nr].l[k]].nl; cl++)
	{
          l=lcmb[surf[nr].l[k]].l[cl];
          if(lcmb[surf[nr].l[k]].o[cl]=='-') flag=-1;
          else flag=1;

          if(flag==1)
          {
            if(cl==0)                     { n=0; flag=line[l].nip-3; }
            else if(cl==lcmb[surf[nr].l[k]].nl-1)    { n=0; flag=line[l].nip; }
            else                         { n=0; flag=line[l].nip-3; }
            do
            {
               n++; nclp++;
            }while(n<flag);
          }
          else
          {
            if(cl==0)                     { n=line[l].nip; flag=3; }
            else if(cl==lcmb[surf[nr].l[k]].nl-1)    { n=line[l].nip; flag=0; }
            else                         { n=line[l].nip; flag=3; }
            while(n>flag)
            {
              n-=3;
              nclp+=3;
            }
          }
	}
        nip+=nclp;
      }
      k++;
    }
    // calloc((nip+xx) NURS_ADD_AMBIG_PNTS points more to cover ambiguous points (should be sufficient)
    //printf("nip:%d\n",nip);
    if( (nurbs[nurbsnr].uv[patch][i] = (GLfloat *)calloc((nip+NURS_ADD_AMBIG_PNTS*2),sizeof(GLfloat) ))==NULL )
    { printf(" ERROR: realloc failure12, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name); return(-1); }
    if( (nurbs[nurbsnr].xyz[patch][i] = (double *)calloc((nip+NURS_ADD_AMBIG_PNTS*3),sizeof(double ) ))==NULL )
    { printf(" ERROR: realloc failure13, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name); return(-1); }
  }
  sem_post(&sem_g);

  /* from here on a trimming attempt is made if necessary and repeated if failed */
  tol_ambig=TOL_AMBIG; 
  k=0;
  for (i=0; i<surf[nr].nc; i++)
  {
    p=0;
    cp=0;
    nip=0;
    for(j=0; j<surf[nr].c[i]; j++)
    {
      /* store the locations of all line-points, they will be projected to the NURBS */
      if(surf[nr].typ[k]=='l')
      {
        l=surf[nr].l[k];
        nip+=line[l].nip;

        if(surf[nr].o[k]=='+')
        {             
          if(j==0)                     { n=0; flag=line[l].nip-3; }
          else if(j==surf[nr].c[i]-1)    { n=0; flag=line[l].nip; }
          else                         { n=0; flag=line[l].nip-3; }
          do
          {
  sem_wait(&sem_g);
            nurbs[nurbsnr].xyz[patch][i][p]=line[l].ip[n++];
  sem_post(&sem_g);
            p++;
#if TEST
	    if(p%3==0)
	      {
        printf("patch%d i%d p%d\n", patch,i,p);
	printf("pnt ! %f %f %f\n", nurbs[nurbsnr].xyz[patch][i][p-3]*scale->w+scale->x,nurbs[nurbsnr].xyz[patch][i][p-2]*scale->w+scale->y,nurbs[nurbsnr].xyz[patch][i][p-1]*scale->w+scale->z);
	      }
#endif
          }while(n<flag);
        }
        else
        {             
          if(j==0)                     { n=line[l].nip; flag=3; }
          else if(j==surf[nr].c[i]-1)    { n=line[l].nip; flag=0; }
          else                         { n=line[l].nip; flag=3; }
          while(n>flag)
          {
  sem_wait(&sem_g);
            nurbs[nurbsnr].xyz[patch][i][p+2]=line[l].ip[--n];
            nurbs[nurbsnr].xyz[patch][i][p+1]=line[l].ip[--n];
            nurbs[nurbsnr].xyz[patch][i][p]=line[l].ip[--n];
  sem_post(&sem_g);
            p+=3;
#if TEST
	    if(p%3==0)
	      {
        printf("patch%d i%d p%d\n", patch,i,p);
	printf("pnt ! %f %f %f\n", nurbs[nurbsnr].xyz[patch][i][p-3]*scale->w+scale->x,nurbs[nurbsnr].xyz[patch][i][p-2]*scale->w+scale->y,nurbs[nurbsnr].xyz[patch][i][p-1]*scale->w+scale->z);
	      }
#endif
          }
        }
      }
      else
      {
        /* first generate an array of internal points of all lines */
        nclp=0;
        for(cl=0; cl<lcmb[surf[nr].l[k]].nl; cl++)
	{
          l=lcmb[surf[nr].l[k]].l[cl];

          if( (xyz = (double *)realloc( (double *)xyz, (nclp+line[l].nip)*sizeof(double ) ))==NULL )
          { printf(" ERROR: realloc failure15, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
            return(-1); }

          if(lcmb[surf[nr].l[k]].o[cl]=='-') flag=-1;
          else flag=1;

          if(flag==1)
          {
            if(cl==0)                     { n=0; flag=line[l].nip-3; }
            else if(cl==lcmb[surf[nr].l[k]].nl-1)    { n=0; flag=line[l].nip; }
            else                         { n=0; flag=line[l].nip-3; }
            do
            {
              xyz[nclp]=line[l].ip[n++];
              nclp++;
            }while(n<flag);
          }
          else
          {
            if(cl==0)                     { n=line[l].nip; flag=3; }
            else if(cl==lcmb[surf[nr].l[k]].nl-1)    { n=line[l].nip; flag=0; }
            else                         { n=line[l].nip; flag=3; }
            while(n>flag)
            {
              xyz[nclp+2]=line[l].ip[--n];
              xyz[nclp+1]=line[l].ip[--n];
              xyz[nclp]=line[l].ip[--n];
              nclp+=3;
            }
          }
	}

        /* then add this points to the line-loop */
        nip+=nclp;

        if(surf[nr].o[k]=='+')
        {             
          if(j==0)                     { n=0; flag=nclp-3; }
          else if(j==surf[nr].c[i]-1)    { n=0; flag=nclp; }
          else                         { n=0; flag=nclp-3; }
          do
          {
  sem_wait(&sem_g);
            nurbs[nurbsnr].xyz[patch][i][p]=xyz[n++];
  sem_post(&sem_g);
            p++;
#if TEST
	    if(p%3==0)
	      {
        printf("#+lcmb patch%d i%d p%d\n", patch,i,p);
	printf("pnt ! %f %f %f\n", nurbs[nurbsnr].xyz[patch][i][p-3]*scale->w+scale->x,nurbs[nurbsnr].xyz[patch][i][p-2]*scale->w+scale->y,nurbs[nurbsnr].xyz[patch][i][p-1]*scale->w+scale->z);
	      }
#endif
          }while(n<flag);
        }
        else
        {             
          if(j==0)                     { n=nclp; flag=3; }
          else if(j==surf[nr].c[i]-1)    { n=nclp; flag=0; }
          else                         { n=nclp; flag=3; }
          while(n>flag)
          {
  sem_wait(&sem_g);
            nurbs[nurbsnr].xyz[patch][i][p+2]=xyz[--n];
            nurbs[nurbsnr].xyz[patch][i][p+1]=xyz[--n];
            nurbs[nurbsnr].xyz[patch][i][p]=xyz[--n];
  sem_post(&sem_g);
            p+=3;
#if TEST
        printf("#-lcmb patch%d i%d p%d\n",k, patch,i,p);
	printf("pnt ! %f %f %f\n", nurbs[nurbsnr].xyz[patch][i][p-3]*scale->w+scale->x,nurbs[nurbsnr].xyz[patch][i][p-2]*scale->w+scale->y,nurbs[nurbsnr].xyz[patch][i][p-1]*scale->w+scale->z);
#endif
          }
        }
      }

      k++;
    }
  sem_wait(&sem_g);
    nurbs[nurbsnr].np[patch][i]=p/3;
  sem_post(&sem_g);
  }
  sem_wait(&sem_g);
  nurbs[nurbsnr].nc[patch]=surf[nr].nc;
  sem_post(&sem_g);

  /* calc u,v coordinates for all trimming-points */
  calcNurbsResolution(nurbsnr);
  i=trimNurbs( nurbsnr, patch, tol_ambig);
  if(i!=0) return(i);

  for (i=0; i<surf[nr].nc; i++)
  {
    lcurve[i]=0;
    /* determine the average point, needed to define the orientation of the curve */
  sem_wait(&sem_g);
    p1[0]=nurbs[nurbsnr].uv[patch][i][0];
    p1[1]=nurbs[nurbsnr].uv[patch][i][1];
    nurbsbuf[0]=nurbs[nurbsnr].np[patch][i]*2;
  sem_post(&sem_g);
   nu[i]+=p1[0];
    nv[i]+=p1[1];
    p1[2]=p2[2]=0.;
    cp=2; do
    {
  sem_wait(&sem_g);
      p2[0]=nurbs[nurbsnr].uv[patch][i][cp++];
      p2[1]=nurbs[nurbsnr].uv[patch][i][cp++];
  sem_post(&sem_g);
      nu[i]+=p2[0];
      nv[i]+=p2[1];
      v_result( p1, p2, p1p2 );
      p1[0]=p2[0];
      p1[1]=p2[1];
      lp1p2=v_betrag( p1p2 );
      lcurve[i]+=lp1p2;
    }while(cp<nurbsbuf[0]);
    nu[i]/=cp/2;
    nv[i]/=cp/2;

    if(lcurve[i]>lmax) { lmax=lcurve[i]; c_outer=i; }
  }
  if(printFlag) printf("outer curve:%d\n",c_outer);


  /* if the outer loop c_outer is not the first one (0) then re-arrange the loops */
  if(c_outer!=0)
  {

    /* which lines are affected? */
    firstl=0; for(n=0; n<c_outer; n++) firstl+=surf[nr].c[n]; lastl=firstl+surf[nr].c[n];

    printf("l:%d 1st:%d last:%d\n",surf[nr].nl,firstl,lastl);
    for(n=0; n<surf[nr].nl-1; n++)
    {
      printf("%d ", surf[nr].l[n]);
    }
    printf("\n");

    if( (linbuf= (int *)malloc( (lastl-firstl+1)*sizeof(int) )) == NULL )
    { printf(" ERROR: realloc failure\n\n"); return(-1); }
    if( (oribuf= (char *)malloc( (lastl-firstl+1)*sizeof(char) )) == NULL )
    { printf(" ERROR: realloc failure\n\n"); return(-1); }

    j=0; for(l=n=surf[nr].nl-1; n>=0; n--)
    {
      if((n<lastl)&&(n>=firstl))
      {
        linbuf[j]=surf[nr].l[n];
        oribuf[j++]=surf[nr].o[n];
      }
      else
      {
        surf[nr].l[l]=surf[nr].l[n];
        surf[nr].o[l--]=surf[nr].o[n];
      }
    }
    j=0; for(n=l; n>=0; n--)
    {
      surf[nr].l[n]=linbuf[j];
      surf[nr].o[n]=oribuf[j++];
    }
    printf("\n");
    j=0; for(n=0; n<surf[nr].nl-1; n++)
    {
      printf("%d ", surf[nr].l[n]);
    }
    printf("\n");
    free(linbuf);
    free(oribuf);
    c_outer=0;
    orientSurf(nr);
    for (i=0; i<surf[nr].nc; i++)
    {
      free(nurbs[nurbsnr].uv[patch][i]);
      free(nurbs[nurbsnr].xyz[patch][i]);
    }
    goto newTrimming;
  }

  /* determine umax, vmax. Used in drawNurs_plot() */
  sem_wait(&sem_g);
  nurbs[nurbsnr].umax[patch]=nurbs[nurbsnr].vmax[patch]=-MAX_INTEGER;
  n=0; while(n<(nurbs[nurbsnr].np[patch][c_outer]*2))
  {
    if(nurbs[nurbsnr].umax[patch]<nurbs[nurbsnr].uv[patch][c_outer][n])
      nurbs[nurbsnr].umax[patch]=nurbs[nurbsnr].uv[patch][c_outer][n];
    n++;  
    if(nurbs[nurbsnr].vmax[patch]<nurbs[nurbsnr].uv[patch][c_outer][n])
      nurbs[nurbsnr].vmax[patch]=nurbs[nurbsnr].uv[patch][c_outer][n];  
    n++; 
  }
  if(printFlag) printf("patch:%d uvw_cp outer curve:%lf %lf %lf maxu:%lf maxv:%lf\n", patch, nu[c_outer], nv[c_outer], lmax, nurbs[nurbsnr].umax[patch], nurbs[nurbsnr].vmax[patch]);
  sem_post(&sem_g);

  /* ORIENT-CHECK */
  /* check if the outer curve is correct oriented for trimming */
  /* the 1st has to be counter-clockwhise and the others clockwhise */
  /* calculate the normal-vector of the outer-curve in the uv space of the nurbs */
  /* if it points in -w direction (lmax<0) then uv of the Nurbs must be changed */
  sem_wait(&sem_g);
  nurbsbuf[0]=nurbs[nurbsnr].nc[patch];
  sem_post(&sem_g);
  for(i=0; i<nurbsbuf[0]; i++)
  {
    p0[0]=nu[i];
    p0[1]=nv[i];
    p0[2]=0.;
    p1[2]=0.;
    p2[2]=0.;
	  //printf("seto curve%d\n pnt ! %f %f\n",i, p0[0],p0[1]);
    n=0; lmax=0.;
  sem_wait(&sem_g);
    nurbsbuf[1]=(nurbs[nurbsnr].np[patch][i]*2)-4;
  sem_post(&sem_g);
    do
    {
      /* add all normals based on uv[n] x uv[n+1] */
  sem_wait(&sem_g);
      p1[0]=nurbs[nurbsnr].uv[patch][i][n++];
      p1[1]=nurbs[nurbsnr].uv[patch][i][n++];
      p2[0]=nurbs[nurbsnr].uv[patch][i][n++];
      p2[1]=nurbs[nurbsnr].uv[patch][i][n++];
  sem_post(&sem_g);
      v_result(p0, p1, p0p1);
      v_result(p0, p2, p0p2);
      v_prod(p0p1,p0p2,vn);
      lmax+=vn[2];
      //printf("curve:%d p1:%lf %lf p2:%lf %lf vn2:%lf lmax:%lf\n",i, p1[0],p1[1], p2[0],p2[1],vn[2], lmax);
      //printf(" pnt ! %f %f\n", p1[0],p1[1]);
    }while(n<nurbsbuf[1]);
    //printf("setc\n# lmax:%lf\n",lmax);

    /* invert lmax for the inner curves for detection purposes*/
    if(i==c_outer)
    {
      /* detect the orientation of the nurbs relative to the surf with the outer loop */
      if(lmax<0) surf[nr].sho='-'; else surf[nr].sho='+';
    }
    else lmax*=-1.;

    if(lmax<0.)
    {
  sem_wait(&sem_g);
      nurbs[nurbsnr].uvflipped[patch][i]=1;
      /* orientation of the curve must be changed (change u,v) */
      if( (knt= (GLdouble *)realloc( (GLdouble *)knt, ((nurbs[nurbsnr].np[patch][i]*2)+1)*sizeof(GLdouble) )) == NULL )
      { printf(" ERROR: realloc failure16, nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name);
      return(-1); }
      for (j=0; j<nurbs[nurbsnr].np[patch][i]*2; j++) knt[j]=nurbs[nurbsnr].uv[patch][i][j];
      n=(nurbs[nurbsnr].np[patch][i]*2)-1; j=0;
      do
      {
        nurbs[nurbsnr].uv[patch][i][j++]=knt[n-1];
        nurbs[nurbsnr].uv[patch][i][j++]=knt[n];
        n-=2;
      }while(n>0);
  sem_post(&sem_g);
    }
    clmax[i]=lmax;
  }

  sem_wait(&sem_g);
  nurbsbuf[0]=nurbs[nurbsnr].nc[patch];
  sem_post(&sem_g);
  for(i=0; i<nurbsbuf[0]; i++)
  {
    lmax=clmax[i];
    /* change the orientation also in the surf-definition */
    if( (surf[nr].sho!='-')&&(surf[nr].sho!='+')) { printf("error sho :%c %d uninitialized in surf:%s. Talk to the programmer!\n", surf[nr].sho,surf[nr].sho,surf[nr].name); exit(0); }
    if(((lmax>0.)&&(surf[nr].sho=='-'))||((lmax<0.)&&(surf[nr].sho=='+')))
    {
      /* which lines are affected? */
      firstl=0; for(n=0; n<i; n++) firstl+=surf[nr].c[n]; lastl=firstl+surf[nr].c[n];

      if( (linbuf= (int *)malloc( (lastl-firstl+1)*sizeof(int) )) == NULL )
      { printf(" ERROR: realloc failure\n\n"); return(-1); }
      if( (oribuf= (char *)malloc( (lastl-firstl+1)*sizeof(char) )) == NULL )
      { printf(" ERROR: realloc failure\n\n"); return(-1); }

      j=0; for(n=firstl; n<lastl; n++)
      {
        linbuf[j]=surf[nr].l[n];
        if(surf[nr].o[n]=='+') oribuf[j++]='-'; else oribuf[j++]='+';
      }
      for(n=firstl; n<lastl; n++)
      {
        j--;
        surf[nr].l[n]=linbuf[j];
        surf[nr].o[n]=oribuf[j];
      }
      free(linbuf);
      free(oribuf);
    }
  }
  free(lcurve);
  free(clmax);
  free(nu);
  free(nv);
  free(knt);
  return(0);
}



int fillNurbsSurf(int nurbsnr, int nr)
{
  int i,j,n,k;
  int nurbsbuf=0, oprod;
  GLint ipuf[2];
  double *fbuf=NULL;
  Nurbs nurb;
  int    patch;

  /* nurbs evaluation into the back-buffer */
  GLfloat *feedbackbuffer=NULL;
  GLint   size_fbb;

  /* the triangulation using GLu func works only with active graphics */
  if(nurbs[nurbsnr].endFlag==0)
  {
    printf(" WARNING: nurbs %s not valid\n",nurbs[nurbsnr].name);
    surf[nr].npgn=0;
    return(-1);
  }
  if(!inpformat) { surf[nr].npgn=0; return(-1); }

  /* check if the nurbs can be handled by the libGLU. */
  /* if not create an temporary approximation. The trias will be later corrected by the original nurbs */  
  if(((nurbs[nurbsnr].u_exp>=gl_max_eval_order)||(nurbs[nurbsnr].v_exp>=gl_max_eval_order)))
  {
    if(printFlag) printf("WARNING: Nurbs:%s of order:%d %d will be redefined. Only %d is supported by the gl-lib.\n", nurbs[nurbsnr].name, nurbs[nurbsnr].u_exp+1, nurbs[nurbsnr].v_exp+1, gl_max_eval_order);
    nurbsbuf=1;

    /* save the original definition in a buffer */
    nurb.u_exp = nurbs[nurbsnr].u_exp;
    nurb.v_exp = nurbs[nurbsnr].v_exp;
    nurb.u_npnt= nurbs[nurbsnr].u_npnt;
    nurb.v_npnt= nurbs[nurbsnr].v_npnt;
    nurb.u_nknt= nurbs[nurbsnr].u_nknt;
    nurb.v_nknt= nurbs[nurbsnr].v_nknt;
    nurb.u_stride= nurbs[nurbsnr].u_stride;
    nurb.v_stride= nurbs[nurbsnr].v_stride;
    if ( (nurb.uknt = (GLfloat *)malloc( (nurbs[nurbsnr].u_nknt+1) * sizeof(GLfloat))) == NULL )
      printf("\n\n ERROR: realloc failed uknt\n\n");
    if ( (nurb.vknt = (GLfloat *)malloc( (nurbs[nurbsnr].v_nknt+1) * sizeof(GLfloat))) == NULL )
      printf("\n\n ERROR: realloc failed vknt\n\n");
    for(i=0; i<nurb.u_nknt; i++) { nurb.uknt[i]=nurbs[nurbsnr].uknt[i]; }
    for(i=0; i<nurb.v_nknt; i++) { nurb.vknt[i]=nurbs[nurbsnr].vknt[i]; }
    if( (nurb.ctlarray = (GLfloat *)malloc( (nurbs[nurbsnr].u_npnt*nurbs[nurbsnr].v_npnt*nurbs[nurbsnr].v_stride+5)*sizeof(GLfloat) )) == NULL )
	  { printf(" ERROR: realloc failure in repairNurbs(), nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name); return(0); }
    for (i=0; i<nurbs[nurbsnr].u_npnt*nurbs[nurbsnr].v_npnt*nurbs[nurbsnr].v_stride+4; i++) nurb.ctlarray[i]=nurbs[nurbsnr].ctlarray[i];

    /* create the approximation (repairNurbs have to be called in a sem block) */
    if(nurbs[nurbsnr].u_exp>=gl_max_eval_order) repairNurbs( nurbsnr, nurbs[nurbsnr].u_exp-gl_max_eval_order+1, 0);
    if(nurbs[nurbsnr].v_exp>=gl_max_eval_order) repairNurbs( nurbsnr, nurbs[nurbsnr].v_exp-gl_max_eval_order+1, 1);

    if(printFlag) printf("nurbs generated and approximated:%s\n",nurbs[nurbsnr].name);
  }
  else nurbsbuf=0;

  glutSetWindow( w1);

  /* now the trimmed region is defined. Render the nurbs-patch into the back-buffer and store */
  /* the poligons inside the surface */
  patch=surf[nr].patch;
  if(printFlag) printf("patch:%d of NURBS:%s patches:%d is now rendered and stored in surf:%s\n", patch, nurbs[nurbsnr].name, nurbs[nurbsnr].patches, surf[nr].name );

  if(patch>=nurbs[nurbsnr].patches)
  {
    i=calcTrimLoops(nurbsnr, nr);
    if(i!=0) { surf[nr].npgn=0; if(i>0) surf[nr].fail=2; return(-1); }
  }
  printf(" glu rendering of surf:%s (should be avoided by changing the line divisions)\n",surf[nr].name);
  // the system libGLU has a bug and might be replaced by the https://archive.mesa3d.org/glu/glu-9.0.0.tar(SGI) used in Makefile_glu
  
  /* disable culling else not all surfs will be filled */
  glGetIntegerv( GL_CULL_FACE_MODE, ipuf );
  glDisable ( GL_CULL_FACE );

  /* malloc the feedbackbuffer, the address will be stored later and reallocated in surf[nr].pgn */
  size_fbb=GL_FEEDBACK_BUFF_SIZE;
  if( (feedbackbuffer= (GLfloat *)malloc(size_fbb*sizeof(GLfloat) )) == NULL )
  { printf(" ERROR: realloc failure, feedbackbuffer to big\n\n");
    return(0); }
  glLoadIdentity();
  glOrtho( -1.*aspectRatio_w1, 1.*aspectRatio_w1, -1., 1., -1, 1. ); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFeedbackBuffer (size_fbb, GL_3D, feedbackbuffer);
  glRenderMode (GL_FEEDBACK);
  /* draw the nurbs, copy from drawNurs_plot() */
  gluNurbsProperty(nurbs[nurbsnr].Nurb, GLU_DISPLAY_MODE, GLU_FILL);
  gluNurbsProperty(nurbs[nurbsnr].Nurb, GLU_CULLING, GL_TRUE);
  gluNurbsProperty(nurbs[nurbsnr].Nurb, GLU_SAMPLING_METHOD, GLU_DOMAIN_DISTANCE );
  gluNurbsProperty(nurbs[nurbsnr].Nurb, GLU_U_STEP,  nurbs[nurbsnr].ustep[patch] );
  gluNurbsProperty(nurbs[nurbsnr].Nurb, GLU_V_STEP,  nurbs[nurbsnr].vstep[patch] );
  gluBeginSurface(nurbs[nurbsnr].Nurb);
  gluNurbsSurface(nurbs[nurbsnr].Nurb,
  nurbs[nurbsnr].u_nknt, nurbs[nurbsnr].uknt,
  nurbs[nurbsnr].v_nknt, nurbs[nurbsnr].vknt,
  nurbs[nurbsnr].u_stride, nurbs[nurbsnr].v_stride,
  nurbs[nurbsnr].ctlarray,
  nurbs[nurbsnr].u_exp+1, nurbs[nurbsnr].v_exp+1,
  nurbs[nurbsnr].type);
  if(printFlag)
  {
    printf("nurbs-param: %d  %f %f  %d %d  %d %d  %d %d  %d  nod:%f %f %f %f arr:%f %f\n", size_fbb,
      nurbs[nurbsnr].ures,
      nurbs[nurbsnr].vres,
      nurbs[nurbsnr].u_nknt,
      nurbs[nurbsnr].v_nknt,
      nurbs[nurbsnr].u_stride, nurbs[nurbsnr].v_stride,
      nurbs[nurbsnr].u_exp+1, nurbs[nurbsnr].v_exp+1,
      nurbs[nurbsnr].type,
      nurbs[nurbsnr].uknt[0],
      nurbs[nurbsnr].uknt[nurbs[nurbsnr].u_nknt-1],
      nurbs[nurbsnr].vknt[0],
      nurbs[nurbsnr].vknt[nurbs[nurbsnr].v_nknt-1],
      nurbs[nurbsnr].ctlarray[0],
      nurbs[nurbsnr].ctlarray[nurbs[nurbsnr].u_npnt*nurbs[nurbsnr].v_npnt]
    );
  }
  
  for(i=0; i<nurbs[nurbsnr].nc[patch]; i++)
  {
    if(printFlag)
    {
      printf("glu input in cgx format u,v,- %d %d %d\n", nurbs[nurbsnr].trimFlag, nurbs[nurbsnr].nc[patch], i);
      n=0; for(j=0; j< nurbs[nurbsnr].np[patch][i]; j++)
      {
        printf(" pnt %d%d %e %e 0.\n",i,j, nurbs[nurbsnr].uv[patch][i][n], nurbs[nurbsnr].uv[patch][i][n+1]);
        n+=2;
        //printf(" pnt %d%d %lf %lf %lf\n",i,j, nurbs[nurbsnr].xyz[patch][i][n], nurbs[nurbsnr].xyz[patch][i][n+1], nurbs[nurbsnr].xyz[patch][i][n+2]);
        //n+=3;
      }
    }
    gluBeginTrim(nurbs[nurbsnr].Nurb);
    gluPwlCurve(nurbs[nurbsnr].Nurb, nurbs[nurbsnr].np[patch][i], nurbs[nurbsnr].uv[patch][i], (GLint)2, GLU_MAP1_TRIM_2);
    gluEndTrim(nurbs[nurbsnr].Nurb);
  }
  
  gluEndSurface(nurbs[nurbsnr].Nurb);
  surf[nr].npgn=glRenderMode (GL_RENDER);

  /* resore the original cull mode */
  if ( ipuf[0] == GL_BACK ) glCullFace ( GL_BACK );
  if ( ipuf[0] == GL_FRONT ) glCullFace ( GL_FRONT );

  if(size_fbb<surf[nr].npgn) 
  {
    printf("ERROR in repSurf: feedbackbuffer:%d to small, increase at least to:%d \n", size_fbb, surf[nr].npgn);
    return(-1);
  }

  /* store data from (GLfloat *)feedbackbuffer into  (double *)surf[nr].pgn */
  if (surf[nr].npgn) free(surf[nr].pgn);

  /* get the address of a new array including the normals */ 
  surf[nr].npgn=adjustFeedBack( surf[nr].npgn, feedbackbuffer, &surf[nr].pgn);
  free(feedbackbuffer);

  /* the triangles must be inverted if the product of the nurbs-ori and surf-ori is '-' */
  if(surf[nr].sho=='-') oprod=-1; else oprod=1;
  if(surf[nr].ori=='-') oprod*=-1; else oprod*=1;
  if(oprod==-1)
  {
    n=0;
    while((surf[nr].npgn-n))
    {
      /* switch interior of surf, code from pickfunktions (l:2560) */
      n++; /* jump over the polygon token (ie.GL_POLYGON_TOKEN) */
      j=surf[nr].pgn[n++];
      surf[nr].pgn[n]*=-1;
      surf[nr].pgn[n+1]*=-1;
      surf[nr].pgn[n+2]*=-1;
      n+=3;
      if ((fbuf = (double *)malloc((j*3)*sizeof(double)) ) == NULL )
		{ printf("\n\nERROR: realloc failure in flip\n\n"); return(0); }
      for(k=0; k<j; k++)
      {
        fbuf[j*3-k*3-3]=surf[nr].pgn[n];
        fbuf[j*3-k*3-2]=surf[nr].pgn[n+1];
        fbuf[j*3-k*3-1]=surf[nr].pgn[n+2];
        n+=3;
      }
      n-=3*j;
      for(k=0; k<j; k++)
      {
        surf[nr].pgn[n]  =fbuf[k*3] ; 
        surf[nr].pgn[n+1]=fbuf[k*3+1];
        surf[nr].pgn[n+2]=fbuf[k*3+2];
        n+=3;
      }
      free(fbuf);
    }
  }

  /* correct the position of the trias if an nurbs-approximation was used and change back to the original nurbs */
  if(nurbsbuf)
  {
    /* restore the original definition */
    nurbs[nurbsnr].u_exp = nurb.u_exp; 
    nurbs[nurbsnr].v_exp = nurb.v_exp;
    nurbs[nurbsnr].u_npnt= nurb.u_npnt;
    nurbs[nurbsnr].v_npnt= nurb.v_npnt;
    nurbs[nurbsnr].u_nknt= nurb.u_nknt;
    nurbs[nurbsnr].v_nknt= nurb.v_nknt;
    nurbs[nurbsnr].u_stride= nurb.u_stride;
    if ( (nurbs[nurbsnr].uknt = (GLfloat *)realloc(  (GLfloat *)nurbs[nurbsnr].uknt, (nurbs[nurbsnr].u_nknt+1) * sizeof(GLfloat))) == NULL )
      printf("\n\n ERROR: realloc failed uknt\n\n");
    if ( (nurbs[nurbsnr].vknt = (GLfloat *)realloc(  (GLfloat *)nurbs[nurbsnr].vknt, (nurbs[nurbsnr].v_nknt+1) * sizeof(GLfloat))) == NULL )
      printf("\n\n ERROR: realloc failed vknt\n\n");
    for(i=0; i<nurbs[nurbsnr].u_nknt; i++) { nurbs[nurbsnr].uknt[i]=nurb.uknt[i]; }
    for(i=0; i<nurbs[nurbsnr].v_nknt; i++) { nurbs[nurbsnr].vknt[i]=nurb.vknt[i]; }
    if( (nurbs[nurbsnr].ctlarray = (GLfloat *)realloc( (GLfloat *)nurbs[nurbsnr].ctlarray, (nurbs[nurbsnr].u_npnt*nurbs[nurbsnr].v_npnt*nurbs[nurbsnr].v_stride+5)*sizeof(GLfloat) )) == NULL )
	    { printf(" ERROR: realloc failure in repairNurbs(), nurbs:%s can not be shaped\n\n", nurbs[nurbsnr].name); return(0); }
    for (i=0; i<nurbs[nurbsnr].u_npnt*nurbs[nurbsnr].v_npnt*4+4; i++) nurbs[nurbsnr].ctlarray[i]=nurb.ctlarray[i];
    free(nurb.ctlarray);
    free(nurb.uknt);
    free(nurb.vknt);

    /* project the vertexes to the original nurbs */
    if(surf[nr].fail!=2) projSurfToNurbs( nurbsnr, surf, nr, 0 );
  }

  return(1);
}



/*  store the polygons in the master-surface for illuminated rendering */
/*  if the surface is nurbs-related: */
/*  - create trimming-points in uv-space for dispaying and meshing trimmed Nurbs-Shapes */
/*  if the surf is BLENDed */
/*  - mesh the surface with tri3 */
/* return 0 if not successfull, else !=0 */
int repSurf(int nr, int renderFlag )
{
  int i;

  int Stmp=-1, sh_buf=-1, nurbsnr=-1,returnVal=0;

  /* check if the surface is not meshable */
  if(surf[nr].o==NULL) return(0);
  if(surf[nr].o[0]==0) return(0);
  if(surf[nr].nc==0) orientSurf(nr);

  /* if its a BLENDed surface */
  if((renderFlag) && ((surf[nr].name != (char *)NULL )&&(surf[nr].sh<=-1)&&(surf[nr].nc>0)))
  {
    if(printFlag) printf("BLEND:%s\n",surf[nr].name);
    // fillBlendedSurf requires already the substitute surfs if repSurf runs in parallel
  if(sem_wait(&sem_rep)) printf("Error in:sem_wait\n");
    fillBlendedSurf(nr);
  if(sem_post(&sem_rep)) printf("Error in:sem_post\n");
  }

  /* if its a NURBS related surface */
  if(( surf[nr].name != (char *)NULL )&&(surf[nr].sh>-1)&&(surf[nr].nc>0))
  {
    /* if shape, generate prelim nurbs */
    if(shape[surf[nr].sh].type>=0) Stmp= surfToNurs(nr);
    if(Stmp>-1)
    {
      sh_buf=surf[nr].sh;
    sem_wait(&sem_g);
      surf[nr].sh=shape_i( nurbs[Stmp].name, 4, Stmp, 0, 0, 0, 0,0,0);
    sem_post(&sem_g);
      if(surf[nr].pgn!=NULL)
      {
        free(surf[nr].pgn); surf[nr].pgn=NULL; surf[nr].npgn=0;
      }
      if(printFlag) printf (" interior changed to Nurbs: %s\n", nurbs[Stmp].name );
    }
    
    if(shape[surf[nr].sh].type==4)
    {
      nurbsnr=shape[surf[nr].sh].p[0];

      /* calculate the xyz and uv values and orientation of the trimming loops of the surface (patch) (might be included in orientSurf() later) */
      i=calcTrimLoops(nurbsnr, nr);
      if(i!=0)
      {
        if(i>0)
	{
          printf(" ERROR: surf:%s could not be trimmed. All points are located on ambiguous edges. Try to fix the geometry manually.\n",surf[nr].name);
          surf[nr].fail=2;
	}
        goto nurbsCouldNotBeTrimmed;
      }

      if(renderFlag)
      {
        returnVal=mesh_tr3u(nr, renderFlag);
        /* check if the surface could be trimmed and rendered */
        if(surf[nr].npgn<1)
        {
	  if(printFlag) printf("WARNING: mesh_tr3u failed surf:%s \n",surf[nr].name);
          /* try to make a mapped mesh */
          /* change the element attribute to mapped mesh */
          surf[nr].eattr=0;
          // fillBlendedSurf requires already the substitute surfs if repSurf runs in parallel
  if(sem_wait(&sem_rep)) printf("Error in:sem_wait\n");
          fillBlendedSurf(nr);
  if(sem_post(&sem_rep)) printf("Error in:sem_post\n");
          if(surf[nr].npgn>0) projSurfToNurbs( nurbsnr, surf, nr, 0 );
          else
	  {
	    if(printFlag) printf(" WARNING: surf:%s could not be rendered\n", surf[nr].name);
            surf[nr].eattr=-1;
            surf[nr].npgn=0;

          nurbsCouldNotBeTrimmed:;  
            /* add surf to special set */
            seta(set_bsur, "s", nr );
            if(returnVal==-2) seta(set_glur, "s", nr);
	  }
        }
      }

      /* restore the pointer to the shape */
      if(sh_buf>-1)
      {
        surf[nr].sh=sh_buf;
    sem_wait(&sem_g);
        for (i=0; i<nurbs[Stmp].u_npnt; i++) delPnt( nurbs[Stmp].v_npnt, nurbs[Stmp].ctlpnt[i] );
        delNurs( 1, &Stmp ); 
    sem_post(&sem_g);
      }

    }
  }
  if(surf[nr].fail==2) return(0);
  return(1);
}



/* create in-between-points in lines for dispaying curves */
/* return 0 if failed */
int repLine(int j )
{
  int k,n;
  double pn[3];


  /* calculation of the inner points of non-straight lines for drawing purposes */
  if( line[j].name != (char *)NULL )
  {
    if ((line[j].ip = (double *)realloc( (double *)line[j].ip, ((line[j].div+1)*3)*sizeof(double)) ) == NULL )
    { printf(" ERROR: realloc failure in rep(), Line:%s can not be shaped\n\n", line[j].name);
      return(0); }

    n=0;
    line[j].ip[n++]=point[line[j].p1].px;
    line[j].ip[n++]=point[line[j].p1].py;
    line[j].ip[n++]=point[line[j].p1].pz;
    if(line[j].typ=='a') for (k=0; k<line[j].div-1; k++)
    {
      arcNodes( j, k,line[j].div, pn );
      line[j].ip[n++]=pn[0];
      line[j].ip[n++]=pn[1];
      line[j].ip[n++]=pn[2];
    }
    else if (line[j].typ=='s') for (k=0; k<line[j].div-1; k++)
    {
      splineNodes( j, k,line[j].div, pn );
      line[j].ip[n++]=pn[0];
      line[j].ip[n++]=pn[1];
      line[j].ip[n++]=pn[2];
    }
    else if (line[j].typ=='n') for (k=0; k<line[j].div-1; k++)
    {
      nurlNodes( j, k,line[j].div, pn );
      line[j].ip[n++]=pn[0];
      line[j].ip[n++]=pn[1];
      line[j].ip[n++]=pn[2];
    }
    else for (k=0; k<line[j].div-1; k++)
    {
      /* must be! For the nurbs trimming all node-points are necessary */
      /* because long lines would not give enough inbetween points */
      straightNodes( j, k,line[j].div, pn );
      line[j].ip[n++]=pn[0];
      line[j].ip[n++]=pn[1];
      line[j].ip[n++]=pn[2];
    }
    line[j].ip[n++]=point[line[j].p2].px;
    line[j].ip[n++]=point[line[j].p2].py;
    line[j].ip[n++]=point[line[j].p2].pz;
    line[j].nip=n;
    /*   
  printf("line[%d].name:%s line[%d].typ:%c line[%d].div:%d line[%d].nip:%d\n"
  , j,line[j].name,j,line[j].typ,j,line[j].div,j, line[j].nip);
    */
  }
  return(1);
}



/* changes linear elem to quadratic formulation or vice versa and generates nodes in the mid of the face for drawing purposes only */
/* w/o parameter it adjusts the midside nodes of quadratic formulated elements */
void fixMidsideNodes( char *setname, char *parameter)
{
  int i,j,n,f,k,n1,n2,nm, nnew[20], nf;
  int nodseq_tr3[]={0,1,1,2,2,0};
  int nodseq_tr6[]={0,3,1,1,4,2,2,5,0};
  int nodseq_qu4[]={0,1,1,2,2,3,3,0};
  int nodseq_qu8[]={0,4,1,1,5,2,2,6,3,3,7,0};
  int nodseq_te4[]={0,1,1,2,2,0, 0,3,1,3,2,3};
  int nodseq_pe6[]={0,1,1,2,2,0, 3,4,4,5,5,3, 0,3,1,4,2,5};
  int nodseq_he8[]={0,1,1,2,2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7};
  int nodseq_te10[]={0,4,1,1,5,2,2,6,0, 0,7,3,1,8,3,2,9,3};
  int nodseq_pe15[]={0,6,1,1,7,2,2,8,0, 3,12,4,4,13,5,5,14,3, 0,9,3,1,10,4,2,11,5};
  int nodseq_he20[]={0,8,1,1,9,2,2,10,3,3,11,0, 4,16,5,5,17,6,6,18,7,7,19,4, 0,12,4,1,13,5,2,14,6,3,15,7};

  int midnod_tr6[]={3,4,5};
  int midnod_qu8[]={4,5,6,7};
  int midnod_pe15[]={6,7,8,9,10,11,12,13,14};
  int midnod_te10[]={4,5,6,7,8,9};
  int midnod_he20[]={8,9,10,11,12,13,14,15,16,17,18,19};

  int setNr, mode=0, *facenod=NULL,ipuf=0, anz_nmax;

  typedef struct {
    int sum, *n2, *nm;
  }N1nm;
  N1nm *n1nm;
 
  setNr=getSetNr(setname);

  if (setNr<0)
  {
    printf (" fixMidsideNodes: set:%s does not exist\n", setname);
    return;
  }
  if (set[setNr].anz_e==0)
  {
    //printf (" fixMidsideNodes: set:%s contains no elements\n", setname);
    return;
  }

  /* remove midside nodes */
  if(compare(parameter,"rem",2)==2)
  {
    printf("store midside nodes from set:%s in set -delete for further manual manipulation\n",set[setNr].name);
    j=pre_seta("-delete", "i", 0 );
    for(k=0; k<set[setNr].anz_e; k++)
    {
      /* go over the dep elem and delete the midside nodes */
      for (k=0; k<set[setNr].anz_e; k++)
      {
        if (e_enqire[set[setNr].elem[k]].type == 4)
        {
          for (n=0; n<12; n++) seta( j, "n", e_enqire[set[setNr].elem[k]].nod[midnod_he20[n]] );
          /* change element def. */
          e_enqire[set[setNr].elem[k]].type =1;
        }
        else if (e_enqire[set[setNr].elem[k]].type == 5)
        {
          for (n=0; n<9; n++) seta( j, "n", e_enqire[set[setNr].elem[k]].nod[midnod_pe15[n]] );
          /* change element def. */
          e_enqire[set[setNr].elem[k]].type =2;
        }
        else if (e_enqire[set[setNr].elem[k]].type == 6)
        {
          //for (n=0; n<6; n++) printf("n:%d\n", e_enqire[set[setNr].elem[k]].nod[midnod_te10[n]] );
          for (n=0; n<6; n++) seta( j, "n", e_enqire[set[setNr].elem[k]].nod[midnod_te10[n]] );
          /* change element def. */
          e_enqire[set[setNr].elem[k]].type =3;
        }
        else if (e_enqire[set[setNr].elem[k]].type == 8)
        {
          for (n=0; n<3; n++) seta( j, "n", e_enqire[set[setNr].elem[k]].nod[midnod_tr6[n]] );
          /* change element def. */
          e_enqire[set[setNr].elem[k]].type =7;
        }
        else if (e_enqire[set[setNr].elem[k]].type == 10)
        {
          for (n=0; n<4; n++) seta( j, "n", e_enqire[set[setNr].elem[k]].nod[midnod_qu8[n]] );
          /* change element def. */
          e_enqire[set[setNr].elem[k]].type =9;
        }
      }
    }
  }

  /* generate new midside nodes for linear elements (qu4) */
  else if(compare(parameter,"gen",2)==2)
  {
    /* create a table for all nodes which points to already created midside nodes */
    if ( (n1nm = (N1nm *)malloc( (anz->nmax+1) * sizeof(N1nm))) == NULL )
    { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
    for (i=0; i<=anz->nmax; i++) n1nm[i].sum=0;
    for (i=0; i<=anz->nmax; i++) n1nm[i].n2=n1nm[i].nm=NULL;
    anz_nmax=anz->nmax;

    /* go over the dep elem and corr midside nodes */
    for (k=0; k<set[setNr].anz_e; k++)
    {
      /* free space for the normal-vectors */
      if (e_enqire[set[setNr].elem[k]].type == 1)       nf=6;  /* HEXA8 */
      else if (e_enqire[set[setNr].elem[k]].type == 2)  nf=6;  /* PENTA6 */
      else if (e_enqire[set[setNr].elem[k]].type == 3)  nf=4;  /* TET4 */
      else if (e_enqire[set[setNr].elem[k]].type == 7)  nf=1;  /* TRI3  */
      else if (e_enqire[set[setNr].elem[k]].type == 9)  nf=2; /* QUAD4 */
      else if (e_enqire[set[setNr].elem[k]].type == 11) nf=1; /* BEAM */
      else nf=0;
      for(i=0; i<nf; i++) free(e_enqire[set[setNr].elem[k]].side[i]);

      if (e_enqire[set[setNr].elem[k]].type == 1)
      {
        for (n=0; n<12; n++)
        {
          nnew[nodseq_he20[n*3]]=  n1=e_enqire[set[setNr].elem[k]].nod[nodseq_he8[n*2]];
          nnew[nodseq_he20[n*3+2]]=  n2=e_enqire[set[setNr].elem[k]].nod[nodseq_he8[n*2+1]];

          /* check if the nm exists already */
          nm=-1;
          for(i=0; i<n1nm[n1].sum; i++) if(n1nm[n1].n2[i]==n2) nm=n1nm[n1].nm[i];
          for(i=0; i<n1nm[n2].sum; i++) if(n1nm[n2].n2[i]==n1) nm=n1nm[n2].nm[i];

          if(nm==-1)
	  {
            /* generate new node */
            nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );

            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].sum++;
            adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[node[nm].nr].nx, 1);
	  }

          nnew[nodseq_he20[n*3+1]]=node[nm].nr;
        }
        /* change element def. */
        e_enqire[set[setNr].elem[k]].type =4;
        for (n=0; n<20; n++) e_enqire[set[setNr].elem[k]].nod[n]=nnew[n];
      }

      else if (e_enqire[set[setNr].elem[k]].type == 2)
      {
        for (n=0; n<9; n++)
        {
          nnew[nodseq_pe15[n*3]]=  n1=e_enqire[set[setNr].elem[k]].nod[nodseq_pe6[n*2]];
          nnew[nodseq_pe15[n*3+2]]=  n2=e_enqire[set[setNr].elem[k]].nod[nodseq_pe6[n*2+1]];

          /* check if the nm exists already */
          nm=-1;
          for(i=0; i<n1nm[n1].sum; i++) if(n1nm[n1].n2[i]==n2) nm=n1nm[n1].nm[i];
          for(i=0; i<n1nm[n2].sum; i++) if(n1nm[n2].n2[i]==n1) nm=n1nm[n2].nm[i];

          if(nm==-1)
	  {
            /* generate new node */
            nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].sum++;
            adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[node[nm].nr].nx, 1);
	  }

          nnew[nodseq_pe15[n*3+1]]=node[nm].nr;
        }
        /* change element def. */
        e_enqire[set[setNr].elem[k]].type =5;
        for (n=0; n<15; n++) e_enqire[set[setNr].elem[k]].nod[n]=nnew[n];
      }


      else if (e_enqire[set[setNr].elem[k]].type == 3)
      {
        for (n=0; n<6; n++)
        {
          nnew[nodseq_te10[n*3]]=  n1=e_enqire[set[setNr].elem[k]].nod[nodseq_te4[n*2]];
          nnew[nodseq_te10[n*3+2]]=  n2=e_enqire[set[setNr].elem[k]].nod[nodseq_te4[n*2+1]];

          /* check if the nm exists already */
          nm=-1;
          for(i=0; i<n1nm[n1].sum; i++) if(n1nm[n1].n2[i]==n2) nm=n1nm[n1].nm[i];
          for(i=0; i<n1nm[n2].sum; i++) if(n1nm[n2].n2[i]==n1) nm=n1nm[n2].nm[i];

          if(nm==-1)
	  {
            /* generate new node */
            nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );

            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].sum++;
            adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[node[nm].nr].nx, 1);
	  }
          nnew[nodseq_te10[n*3+1]]=node[nm].nr;
        }
        /* change element def. */
        e_enqire[set[setNr].elem[k]].type =6;
        for (n=0; n<10; n++) e_enqire[set[setNr].elem[k]].nod[n]=nnew[n];
      }

      else if (e_enqire[set[setNr].elem[k]].type == 7)
      {
        for (n=0; n<3; n++)
        {
          nnew[nodseq_tr6[n*3]]=  n1=e_enqire[set[setNr].elem[k]].nod[nodseq_tr3[n*2]];
          nnew[nodseq_tr6[n*3+2]]=  n2=e_enqire[set[setNr].elem[k]].nod[nodseq_tr3[n*2+1]];

          /* check if the nm exists already */
          nm=-1;
          for(i=0; i<n1nm[n1].sum; i++) if(n1nm[n1].n2[i]==n2) nm=n1nm[n1].nm[i];
          for(i=0; i<n1nm[n2].sum; i++) if(n1nm[n2].n2[i]==n1) nm=n1nm[n2].nm[i];

          if(nm==-1)
	  {
            /* generate new node */
            nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );

            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].sum++;
            adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[node[nm].nr].nx, 1);
	  }

          nnew[nodseq_tr6[n*3+1]]=node[nm].nr;
        }
        /* change element def. */
        e_enqire[set[setNr].elem[k]].type =8;
        for (n=0; n<6; n++) e_enqire[set[setNr].elem[k]].nod[n]=nnew[n];
      }

      else if (e_enqire[set[setNr].elem[k]].type == 9)
      {
        for (n=0; n<4; n++)
        {
          nnew[nodseq_qu8[n*3]]=  n1=e_enqire[set[setNr].elem[k]].nod[nodseq_qu4[n*2]];
          nnew[nodseq_qu8[n*3+2]]=  n2=e_enqire[set[setNr].elem[k]].nod[nodseq_qu4[n*2+1]];

          /* check if the nm exists already */
          nm=-1;
          for(i=0; i<n1nm[n1].sum; i++) if(n1nm[n1].n2[i]==n2) nm=n1nm[n1].nm[i];
          for(i=0; i<n1nm[n2].sum; i++) if(n1nm[n2].n2[i]==n1) nm=n1nm[n2].nm[i];

          if(nm==-1)
	  {
            /* generate new node */
            nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );

            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].sum++;
            adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[node[nm].nr].nx, 1);
	  }

          nnew[nodseq_qu8[n*3+1]]=node[nm].nr;
        }
        /* change element def. */
        e_enqire[set[setNr].elem[k]].type =10;
        for (n=0; n<8; n++) e_enqire[set[setNr].elem[k]].nod[n]=nnew[n];
      }

      /* space for the normal-vectors */
      if (e_enqire[set[setNr].elem[k]].type == 1)       nf=6;  /* HEXA8 */
      else if (e_enqire[set[setNr].elem[k]].type == 2)  nf=6;  /* PENTA6 */
      else if (e_enqire[set[setNr].elem[k]].type == 3)  nf=4;  /* TET4 */
      else if (e_enqire[set[setNr].elem[k]].type == 4)  nf=48; /* HEXA20 */
      else if (e_enqire[set[setNr].elem[k]].type == 5)  nf=48; /* PENTA15 */
      else if (e_enqire[set[setNr].elem[k]].type == 6)  nf=16; /* TET10 */
      else if (e_enqire[set[setNr].elem[k]].type == 7)  nf=1;  /* TRI3  */
      else if (e_enqire[set[setNr].elem[k]].type == 8)  nf=4; /* TRI6  */
      else if (e_enqire[set[setNr].elem[k]].type == 9)  nf=2; /* QUAD4 */
      else if (e_enqire[set[setNr].elem[k]].type == 10) nf=8; /* QUAD8 */
      else if (e_enqire[set[setNr].elem[k]].type == 11) nf=1; /* BEAM */
      else if (e_enqire[set[setNr].elem[k]].type == 12) nf=1; /* BEAM3 */
  
      if((e_enqire[set[setNr].elem[k]].side=(double **)realloc(e_enqire[set[setNr].elem[k]].side, (nf)*sizeof(double *)))==NULL)
        printf("\n\n ERROR: realloc failed\n\n" );
      for(i=0; i<nf; i++)
      {
        if((e_enqire[set[setNr].elem[k]].side[i]=(double *)malloc((3)*sizeof(double)))==NULL)
          printf("\n\n ERROR: malloc failed\n\n" );
      }
    }
    for (i=0; i<=anz_nmax; i++) { free(n1nm[i].n2); free(n1nm[i].nm); }
    free(n1nm);
  }

  /* adjust midside nodes */
  else
  {
    if( (facenod=(int *)calloc( (anz->nmax+1),sizeof(int) ) )==NULL)
    { printf(" ERROR: realloc failure in fixMidsideNodes\n\n" ); return; }
    for (f=0; f<anz->nmax+1; f++) facenod[f]=0;
    
    /* midside nodes on faces are not lineary readjusted. They stay on the orig curvature. */
    if(compare(parameter,"lin",3)==3)
    {
      mode=1;
      for (f=0; f<anz->f; f++)
      {
        if (face[f].type == 7) ipuf = 3;  /* TRI3 */
        else if (face[f].type== 8) ipuf = 6;  /* TRI6  */
        else if (face[f].type == 9) ipuf = 4;  /* QU4 */
        else if (face[f].type == 10) ipuf = 8;  /* QU8 */
        else ipuf=0;
        for( i=0; i<ipuf; i++)
        {
          facenod[face[f].nod[i]]=1;
        }
      }
    }
    /* all midside nodes are strictly halfway between the corner nodes. */
    else if(compare(parameter,"force",3)==3) mode=1;
    /* all midside nodes stay on the arc through the 3 nodes. */
    else mode=0;

    for(k=0; k<set[setNr].anz_e; k++)
    {
      /* go over the dep elem and corr midside nodes */
      for (k=0; k<set[setNr].anz_e; k++)
      {
        if      (e_enqire[set[setNr].elem[k]].type == 4)
        {
          for (n=0; n<12; n++)
          {
            n1=e_enqire[set[setNr].elem[k]].nod[nodseq_he20[n*3]];
            nm=e_enqire[set[setNr].elem[k]].nod[nodseq_he20[n*3+1]];
            n2=e_enqire[set[setNr].elem[k]].nod[nodseq_he20[n*3+2]];
            if((mode)&&(facenod[nm]==1)) adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, 0);
            else adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, mode);
          }
        }
        else if (e_enqire[set[setNr].elem[k]].type == 5)
        {
          for (n=0; n<9; n++)
          {
            n1=e_enqire[set[setNr].elem[k]].nod[nodseq_pe15[n*3]];
            nm=e_enqire[set[setNr].elem[k]].nod[nodseq_pe15[n*3+1]];
            n2=e_enqire[set[setNr].elem[k]].nod[nodseq_pe15[n*3+2]];
            if((mode)&&(facenod[nm]==1)) adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, 0); 
            else adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, mode);
          }
        }
        else if (e_enqire[set[setNr].elem[k]].type == 6)
        {
          for (n=0; n<6; n++)
          {
            n1=e_enqire[set[setNr].elem[k]].nod[nodseq_te10[n*3]];
            nm=e_enqire[set[setNr].elem[k]].nod[nodseq_te10[n*3+1]];
            n2=e_enqire[set[setNr].elem[k]].nod[nodseq_te10[n*3+2]];
            if((mode)&&(facenod[nm]==1)) adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, 0); 
            else adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, mode);
          }
        }
        else if (e_enqire[set[setNr].elem[k]].type == 8)
        {
          for (n=0; n<3; n++)
          {
            n1=e_enqire[set[setNr].elem[k]].nod[nodseq_tr6[n*3]];
            nm=e_enqire[set[setNr].elem[k]].nod[nodseq_tr6[n*3+1]];
            n2=e_enqire[set[setNr].elem[k]].nod[nodseq_tr6[n*3+2]];
            if((mode)&&(facenod[nm]==1)) adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, 0); 
            else adjustMidsideNode( &node[n1].nx, &node[n2].nx, &node[nm].nx, mode);
          }
        }
      }
    }
  }
  free(facenod);
}



void posMidsideNodes(Nodes *node)
{
  int i, n, n1,n2;

      for ( i=0; i<anz->e; i++ )
      {
        if(e_enqire[e_enqire[i].nr].type==4)
        {
          for (n=0; n<3; n++)  
          {
          node[e_enqire[e_enqire[i].nr].nod[20+n]].nx =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n]].nx+node[e_enqire[e_enqire[i].nr].nod[1+n]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[5+n]].nx+node[e_enqire[e_enqire[i].nr].nod[4+n]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[8+n]].nx+node[e_enqire[e_enqire[i].nr].nod[13+n]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[16+n]].nx+node[e_enqire[e_enqire[i].nr].nod[12+n]].nx) ;

          node[e_enqire[e_enqire[i].nr].nod[20+n]].ny =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n]].ny+node[e_enqire[e_enqire[i].nr].nod[1+n]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[5+n]].ny+node[e_enqire[e_enqire[i].nr].nod[4+n]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[8+n]].ny+node[e_enqire[e_enqire[i].nr].nod[13+n]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[16+n]].ny+node[e_enqire[e_enqire[i].nr].nod[12+n]].ny) ;

          node[e_enqire[e_enqire[i].nr].nod[20+n]].nz =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n]].nz+node[e_enqire[e_enqire[i].nr].nod[1+n]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[5+n]].nz+node[e_enqire[e_enqire[i].nr].nod[4+n]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[8+n]].nz+node[e_enqire[e_enqire[i].nr].nod[13+n]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[16+n]].nz+node[e_enqire[e_enqire[i].nr].nod[12+n]].nz) ;
          }
          node[e_enqire[e_enqire[i].nr].nod[23  ]].nx =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[3]].nx+node[e_enqire[e_enqire[i].nr].nod[0]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[4]].nx+node[e_enqire[e_enqire[i].nr].nod[7]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[11]].nx+node[e_enqire[e_enqire[i].nr].nod[12]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[19]].nx+node[e_enqire[e_enqire[i].nr].nod[15]].nx) ;

          node[e_enqire[e_enqire[i].nr].nod[23  ]].ny =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[3]].ny+node[e_enqire[e_enqire[i].nr].nod[0]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[4]].ny+node[e_enqire[e_enqire[i].nr].nod[7]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[11]].ny+node[e_enqire[e_enqire[i].nr].nod[12]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[19]].ny+node[e_enqire[e_enqire[i].nr].nod[15]].ny) ;

          node[e_enqire[e_enqire[i].nr].nod[23  ]].nz =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[3]].nz+node[e_enqire[e_enqire[i].nr].nod[0]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[4]].nz+node[e_enqire[e_enqire[i].nr].nod[7]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[11]].nz+node[e_enqire[e_enqire[i].nr].nod[12]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[19]].nz+node[e_enqire[e_enqire[i].nr].nod[15]].nz) ;
          for (n=0; n<2; n++) 
          {
          n1=n*4;
          n2=n*8;
          node[e_enqire[e_enqire[i].nr].nod[24+n  ]].nx =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n1]].nx+node[e_enqire[e_enqire[i].nr].nod[1+n1]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[2+n1]].nx+node[e_enqire[e_enqire[i].nr].nod[3+n1]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[8+n2]].nx+node[e_enqire[e_enqire[i].nr].nod[9+n2]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[10+n2]].nx+node[e_enqire[e_enqire[i].nr].nod[11+n2]].nx) ;

          node[e_enqire[e_enqire[i].nr].nod[24+n  ]].ny =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n1]].ny+node[e_enqire[e_enqire[i].nr].nod[1+n1]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[2+n1]].ny+node[e_enqire[e_enqire[i].nr].nod[3+n1]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[8+n2]].ny+node[e_enqire[e_enqire[i].nr].nod[9+n2]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[10+n2]].ny+node[e_enqire[e_enqire[i].nr].nod[11+n2]].ny) ;

          node[e_enqire[e_enqire[i].nr].nod[24+n  ]].nz =-0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n1]].nz+node[e_enqire[e_enqire[i].nr].nod[1+n1]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[2+n1]].nz+node[e_enqire[e_enqire[i].nr].nod[3+n1]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[8+n2]].nz+node[e_enqire[e_enqire[i].nr].nod[9+n2]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[10+n2]].nz+node[e_enqire[e_enqire[i].nr].nod[11+n2]].nz) ;
          }
        }
        if(e_enqire[e_enqire[i].nr].type==5)
        {
          for (n=0; n<2; n++)  /* create new nodes in center of areas */
          {
          node[e_enqire[e_enqire[i].nr].nod[15+n]].nx = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n]].nx+node[e_enqire[e_enqire[i].nr].nod[1+n]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[4+n]].nx+node[e_enqire[e_enqire[i].nr].nod[3+n]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[6+n]].nx+node[e_enqire[e_enqire[i].nr].nod[10+n]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[12+n]].nx+node[e_enqire[e_enqire[i].nr].nod[ 9+n]].nx) ;
  
          node[e_enqire[e_enqire[i].nr].nod[15+n]].ny = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n]].ny+node[e_enqire[e_enqire[i].nr].nod[1+n]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[4+n]].ny+node[e_enqire[e_enqire[i].nr].nod[3+n]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[6+n]].ny+node[e_enqire[e_enqire[i].nr].nod[10+n]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[12+n]].ny+node[e_enqire[e_enqire[i].nr].nod[ 9+n]].ny) ;
  
          node[e_enqire[e_enqire[i].nr].nod[15+n]].nz = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0+n]].nz+node[e_enqire[e_enqire[i].nr].nod[1+n]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[4+n]].nz+node[e_enqire[e_enqire[i].nr].nod[3+n]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[6+n]].nz+node[e_enqire[e_enqire[i].nr].nod[10+n]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[12+n]].nz+node[e_enqire[e_enqire[i].nr].nod[ 9+n]].nz) ;
          }
  
          /* create  new node in center of area3 */
          node[e_enqire[e_enqire[i].nr].nod[17  ]].nx = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[2]].nx+node[e_enqire[e_enqire[i].nr].nod[0]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[3]].nx+node[e_enqire[e_enqire[i].nr].nod[5]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[ 8]].nx+node[e_enqire[e_enqire[i].nr].nod[ 9]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[14]].nx+node[e_enqire[e_enqire[i].nr].nod[11]].nx) ;
  
          node[e_enqire[e_enqire[i].nr].nod[17  ]].ny = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[2]].ny+node[e_enqire[e_enqire[i].nr].nod[0]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[3]].ny+node[e_enqire[e_enqire[i].nr].nod[5]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[ 8]].ny+node[e_enqire[e_enqire[i].nr].nod[ 9]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[14]].ny+node[e_enqire[e_enqire[i].nr].nod[11]].ny) ;
  
          node[e_enqire[e_enqire[i].nr].nod[17  ]].nz = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[2]].nz+node[e_enqire[e_enqire[i].nr].nod[0]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[3]].nz+node[e_enqire[e_enqire[i].nr].nod[5]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[ 8]].nz+node[e_enqire[e_enqire[i].nr].nod[ 9]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[14]].nz+node[e_enqire[e_enqire[i].nr].nod[11]].nz) ;
 
          /* create  new node in center of area4 */
          node[e_enqire[e_enqire[i].nr].nod[18  ]].nx = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0]].nx+node[e_enqire[e_enqire[i].nr].nod[2]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[1]].nx+node[e_enqire[e_enqire[i].nr].nod[0]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[ 8]].nx+node[e_enqire[e_enqire[i].nr].nod[ 7]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[ 6]].nx+node[e_enqire[e_enqire[i].nr].nod[ 0]].nx) ;
  
          node[e_enqire[e_enqire[i].nr].nod[18  ]].ny = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0]].ny+node[e_enqire[e_enqire[i].nr].nod[2]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[1]].ny+node[e_enqire[e_enqire[i].nr].nod[0]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[ 8]].ny+node[e_enqire[e_enqire[i].nr].nod[ 7]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[ 6]].ny+node[e_enqire[e_enqire[i].nr].nod[ 0]].ny) ;
  
          node[e_enqire[e_enqire[i].nr].nod[18  ]].nz = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0]].nz+node[e_enqire[e_enqire[i].nr].nod[2]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[1]].nz+node[e_enqire[e_enqire[i].nr].nod[0]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[ 8]].nz+node[e_enqire[e_enqire[i].nr].nod[ 7]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[ 6]].nz+node[e_enqire[e_enqire[i].nr].nod[ 0]].nz) ;
  
          /* create  new node in center of area5 */
          node[e_enqire[e_enqire[i].nr].nod[19  ]].nx = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[3]].nx+node[e_enqire[e_enqire[i].nr].nod[4]].nx    +
          node[e_enqire[e_enqire[i].nr].nod[5]].nx+node[e_enqire[e_enqire[i].nr].nod[3]].nx )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[12]].nx+node[e_enqire[e_enqire[i].nr].nod[13]].nx   +
          node[e_enqire[e_enqire[i].nr].nod[14]].nx+node[e_enqire[e_enqire[i].nr].nod[ 3]].nx) ;
  
          node[e_enqire[e_enqire[i].nr].nod[19  ]].ny = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[3]].ny+node[e_enqire[e_enqire[i].nr].nod[4]].ny    +
          node[e_enqire[e_enqire[i].nr].nod[5]].ny+node[e_enqire[e_enqire[i].nr].nod[3]].ny )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[12]].ny+node[e_enqire[e_enqire[i].nr].nod[13]].ny   +
          node[e_enqire[e_enqire[i].nr].nod[14]].ny+node[e_enqire[e_enqire[i].nr].nod[ 3]].ny) ;
  
          node[e_enqire[e_enqire[i].nr].nod[19  ]].nz = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[3]].nz+node[e_enqire[e_enqire[i].nr].nod[4]].nz    +
          node[e_enqire[e_enqire[i].nr].nod[5]].nz+node[e_enqire[e_enqire[i].nr].nod[3]].nz )  + 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[12]].nz+node[e_enqire[e_enqire[i].nr].nod[13]].nz   +
          node[e_enqire[e_enqire[i].nr].nod[14]].nz+node[e_enqire[e_enqire[i].nr].nod[ 3]].nz) ;
        }
        if(e_enqire[e_enqire[i].nr].type==10)
        {
          node[e_enqire[e_enqire[i].nr].nod[ 8]].nx = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0]].nx+node[e_enqire[e_enqire[i].nr].nod[1]].nx  +
          node[e_enqire[e_enqire[i].nr].nod[3]].nx+node[e_enqire[e_enqire[i].nr].nod[2]].nx )+ 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[4]].nx+node[e_enqire[e_enqire[i].nr].nod[6]].nx  +
          node[e_enqire[e_enqire[i].nr].nod[7]].nx+node[e_enqire[e_enqire[i].nr].nod[5]].nx) ;

          node[e_enqire[e_enqire[i].nr].nod[ 8]].ny = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0]].ny+node[e_enqire[e_enqire[i].nr].nod[1]].ny  +
          node[e_enqire[e_enqire[i].nr].nod[3]].ny+node[e_enqire[e_enqire[i].nr].nod[2]].ny )+ 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[4]].ny+node[e_enqire[e_enqire[i].nr].nod[6]].ny  +
          node[e_enqire[e_enqire[i].nr].nod[7]].ny+node[e_enqire[e_enqire[i].nr].nod[5]].ny) ;

          node[e_enqire[e_enqire[i].nr].nod[ 8]].nz = -0.25* (
          node[e_enqire[e_enqire[i].nr].nod[0]].nz+node[e_enqire[e_enqire[i].nr].nod[1]].nz  +
          node[e_enqire[e_enqire[i].nr].nod[3]].nz+node[e_enqire[e_enqire[i].nr].nod[2]].nz )+ 0.5*(
          node[e_enqire[e_enqire[i].nr].nod[4]].nz+node[e_enqire[e_enqire[i].nr].nod[6]].nz  +
          node[e_enqire[e_enqire[i].nr].nod[7]].nz+node[e_enqire[e_enqire[i].nr].nod[5]].nz) ;
	}  
      }
}
  


/* generates or fixes nodes in the mid of the element face for drawing purposes only */
/* flag true (1) if new nodes must be allocated */
void adjustDrawNodes(int flag)
{
  int i,j,n, lc;

  if(flag)
  {
    /* nodes of -qcut were placed behind the last address of the midside nodes, delete the elems */
    zap(specialset->etmp);

    /* new number of original nodes */
    anz->orignmax = anz->nmax;
    anz->orign = anz->n;

    /* allocate midside nodes */
    n=0;
    for ( i=0; i<anz->e; i++ )
    {
      switch(e_enqire[e_enqire[i].nr].type)
      {
        case 4:
	  n+=6;
          for(j=0; j<6; j++) e_enqire[e_enqire[i].nr].nod[j+20]=++anz->nmax;
	break;
        case 5:
	  n+=5;
          for(j=0; j<5; j++) e_enqire[e_enqire[i].nr].nod[j+15]=++anz->nmax;
	break;
        case 10:
	  n++;
          e_enqire[e_enqire[i].nr].nod[8]=++anz->nmax;
	break;
      }
    }
    if ((node = (Nodes *)realloc( (Nodes *)node, (anz->nmax+1)*sizeof(Nodes)) ) == NULL )
    { errMsg("ERROR: realloc failure in nod, node:%d not installed\n", anz->nmax); return; }

    anz->nmax = anz->orignmax;
    for (i=0; i<n; i++)
    {
      node[anz->n].nr=++anz->nmax;
      node[node[anz->n].nr].indx=anz->n;
      node[node[anz->n].nr].pflag=1;
      anz->n++;
    }

    /* extend the lc by the new node */
    for (lc=0; lc<anz->l; lc++)
    {
      if (lcase[lc].loaded)
      {
        for(i=0; i<lcase[lc].ncomps; i++)
        {
          if ( (lcase[lc].dat[i] = (float *)realloc(lcase[lc].dat[i], (anz->nmax+1) * sizeof(float))) == NULL )
            printf("\n\n ERROR: realloc failure nod\n\n" );
	  for(j=1; j<=n; j++ ) lcase[lc].dat[i][anz->orignmax+j]=0.;
        }
      }
    }
  }
  posMidsideNodes(node);
  //updateDispLists();
}



int write2stack(int n, char **parameter)
{
  int i;
  if(!valuestackFlag) return(-1); 

  if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
  { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
  for(i=0; i<n; i++)
  {
    if ((valuestack[valuestack_ptr] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    sprintf(valuestack[valuestack_ptr++],"%s", parameter[n-1-i] );
  }
  printf(" %d values in inverse order written to stack\n", n);
  return(n); 
}



int enquireEntities(char *string)
{
  int i,j,k,l, m,n, args;
  static int enq_nr=0;
  int ico[3], setPos=-1, setNr=-1, trgtSet=-1, nval=0, valFlag=0;
  int csys[3];
  char dat[9][MAX_LINE_LENGTH], filenam[MAX_LINE_LENGTH], mode='i';
  double vco[3], tol=0., val=0, nx=0,ny,nz, dx, dy, dz, nr=0, nf, dr, df, rval=0, value=0.;
  double pr[3], pc[3];
  Rsort *rsort=NULL;
  FILE *handle;

  // enq set trgt_set rec|cx|cy|cz|set 3vals|setname ('-' == all possible vals, r+fi+z) tol i|a|h|l [value] (9 args)
  args=sscanf(string,"%s %s %s %s %s %s %s %s %s", dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], dat[6], dat[7], dat[8]);

  if(anz->l)
  {
    /* check if the data of the specified lcase (Dataset) are already available */
    if (!lcase[cur_lc].loaded)
    {
      if( pre_readfrdblock(copiedNodeSets , cur_lc, anz, node, lcase )==-1) 
      {
        printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", cur_lc+1); 
        return(0);
      }
      calcDatasets( cur_lc, anz, node, lcase );
      recompileEntitiesInMenu(cur_lc);
    }
  }

  setNr=getSetNr(dat[0]);
  if(setNr<0)
  {
    printf("ERROR: set:%s does not exist\n", dat[0]);
    sprintf(parameter[0], "ERROR: set %s does not exist", dat[0]);
    write2stack(1, parameter);
    return(0);
  }

  trgtSet=getSetNr(dat[1]);
  if(trgtSet<0)
  {
    trgtSet=pre_seta(dat[1], "i", 0);
  }

  /* get the coordinates */

  /* based on a node or point in a set */
  if(dat[2][0]=='s')
  {
    setPos=getSetNr(dat[3]);
    if(setPos<0)
    {
      printf(" ERROR: Specified set:%s does not exist\n", dat[3]);
      return(0);
    }
    if((!set[setPos].anz_n)&&(!set[setPos].anz_p))
    {
      printf(" ERROR: Specified set:%s contains no node or point\n", dat[3]);
      return(0);
    }
    tol=atof(dat[4]);
    if(args==7) { value=atof(dat[6]); valFlag=1; }
    if(args>=6) mode=dat[5][0];
    else mode='i';
  }
  /* based on coordinates */
  else
  {
    for(i=0; i<3; i++)   if(dat[3+i][0]!='_') { vco[i]=atof(dat[3+i]); ico[i]=1; }  else { ico[i]=0; }

    tol=atof(dat[6]);
    if(args==9) { value=atof(dat[8]); valFlag=1; }
    if(args>=8) mode=dat[7][0];
    else
    {
      for(i=0; i<3; i++) if(!ico[i]) mode='a';
    }
  }
  printf("mode:%c tol:%f\n", mode, tol);

  descalNodes ( anz->n, node, scale );
  descalPoints( anzGeo->p, point, scale);


  if(set[setNr].anz_n)
  {
    /* calculate dr of all nodes and sort the indexes according to distance**2 (rsort[i].r) */ 
    if ( (rsort = (Rsort *)malloc( (set[setNr].anz_n+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  
    for(i=0; i<set[setNr].anz_n; i++)
    {
      n=set[setNr].node[i];
      if(dat[2][0]=='s')
      {
        if(set[setPos].anz_n)
        {
          dx=node[set[setPos].node[0]].nx-node[n].nx;
          dy=node[set[setPos].node[0]].ny-node[n].ny;
          dz=node[set[setPos].node[0]].nz-node[n].nz;
	}
        else if(set[setPos].anz_p)
        {
          dx=point[set[setPos].pnt[0]].px-node[n].nx;
          dy=point[set[setPos].pnt[0]].py-node[n].ny;
          dz=point[set[setPos].pnt[0]].pz-node[n].nz;
	}
        else return(0);
        rsort[i].r=dx*dx+dy*dy+dz*dz;
      }
      else if(dat[2][0]=='r')
      {
        if(ico[0]) dx=vco[0]-node[n].nx; else dx=0.;
        if(ico[1]) dy=vco[1]-node[n].ny; else dy=0.;
        if(ico[2]) dz=vco[2]-node[n].nz; else dz=0.;
        rsort[i].r=dx*dx+dy*dy+dz*dz;
      }
      else if((dat[2][0]=='c')&&(dat[2][2]=='l'))
      {
        printf(" WARNING: The 'cyl x|y|z <r> <z>' option was replaced by 'cx|cy|cz <r> <fi> <z>' and will be deleted in future. Please consult the manual\n"); 
        switch(dat[3][0])
        {
          case 'x':
          {
            nr=sqrt(node[n].nz*node[n].nz+node[n].ny*node[n].ny);
            nx=node[n].nx;
          }
          break;
          case 'y':
          {
            nr=sqrt(node[n].nz*node[n].nz+node[n].nx*node[n].nx);
            nx=node[n].ny;
          }
          break;
          case 'z':
          {
            nr=sqrt(node[n].ny*node[n].ny+node[n].nx*node[n].nx);
            nx=node[n].nz;
          }
          break;
        }
        if(ico[1]) dr=vco[1]-nr; else dr=0.;
        if(ico[2]) dx=vco[2]-nx; else dx=0.;
        rsort[i].r=dr*dr+dx*dx;
	//printf("n:%d dr:%f dx:%f\n", n,dr, dx);
      }
      else if(dat[2][0]=='c')
      {
        if( dat[2][1]=='x') j=0;
        else if( dat[2][1]=='y') j=1;
        else if( dat[2][1]=='z') j=2;
        else {  errMsg("\n\n ERROR: axis unknown\n"); return(0); };
        pr[0]=node[n].nx;
        pr[1]=node[n].ny;
        pr[2]=node[n].nz;
        if( v_rec2cyl( pr, j, csys, pc) ==-3) { rsort[i].r=MAX_FLOAT; continue; }
        nr=pc[0];
        nf=pc[1];
        nx=pc[2];

        if(ico[0]) dr=vco[0]-nr; else dr=0.;
        //if(ico[1]) df=vco[1]-nf; else df=0.;
        // user input was in degree:
        if(ico[1]) df=vco[1]*0.0174532925194-nf; else df=0.;
        if(ico[2]) dx=vco[2]-nx; else dx=0.;
        /* df is in rad but needs mm */
        df*=nr;
        rsort[i].r=dr*dr+df*df+dx*dx;
	//printf("n:%d dr:%f df:%f[rad] dx:%f d**2:%f\n", n,dr,df,dx, rsort[i].r);
      }
      else
      {
        printf("parameter not recognized:%s\n", dat[2]);
        break;
      }
      rsort[i].i=n;
    }
    qsort( rsort, set[setNr].anz_n, sizeof(Rsort), (void *)compareRsort );
  
    switch(mode)
    {
      case 'i':
      {
        if(rsort[0].r<=tol*tol)
        { 
	  //printf("n:%d r:%f\n",  rsort[0].i, rsort[0].r);
          sprintf(parameter[0],"%d", rsort[0].i);
          sprintf(parameter[1],"%e", rsort[0].r);
          write2stack(2, parameter);
          seta(trgtSet, "n", rsort[0].i);
	}
      }
      break;
      case 'a':
      {
        for (i=0; i<set[setNr].anz_n; i++)
        {
          if(rsort[i].r>tol*tol) break;
	  //printf("n:%d r:%f\n",  rsort[i].i, rsort[i].r);
          sprintf(parameter[0],"%d", rsort[i].i);
          sprintf(parameter[1],"%e", rsort[i].r);
          write2stack(2, parameter);
          seta(trgtSet, "n", rsort[i].i);
        }
      }
      break;
      case 'h':
      {
        if(valFlag)
	{
          /* search values above value in range */
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(rsort[i].r>tol*tol) break;
            if(lcase[cur_lc].dat[cur_entity][rsort[i].i]>=value)
            {
              val=lcase[cur_lc].dat[cur_entity][rsort[i].i]; nval=rsort[i].i; rval=rsort[i].r;
              seta(trgtSet, "n", nval);
              printf(" node:%d value:%lf dist:%lf\n", nval, val, sqrt(rval));
              sprintf(parameter[0],"%d", nval);
              sprintf(parameter[1],"%e", val);
              sprintf(parameter[2],"%e", sqrt(rval));
              write2stack(3, parameter);
            }  
          }
	}
        else
	{
          /* search the highest value in range */
          val=-MAX_FLOAT;
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(rsort[i].r>tol*tol) break;
            if(val<lcase[cur_lc].dat[cur_entity][rsort[i].i]) { val=lcase[cur_lc].dat[cur_entity][rsort[i].i]; nval=rsort[i].i; rval=rsort[i].r; }  
          }
          if(nval)
          { 
            seta(trgtSet, "n", nval);
            printf(" node:%d value:%lf dist:%lf\n\n", nval, val, sqrt(rval));
            sprintf(parameter[0],"%d", nval);
            sprintf(parameter[1],"%e", val);
            sprintf(parameter[2],"%e", sqrt(rval));
            write2stack(3, parameter);
          }
          else printf("\n found no node in range\n\n");
	}
      }
      break;
      case 'l':
      { 
        if(valFlag)
	{
          /* search values below value in range */
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(rsort[i].r>tol*tol) break;
            if(lcase[cur_lc].dat[cur_entity][rsort[i].i]<=value)
            {
              val=lcase[cur_lc].dat[cur_entity][rsort[i].i]; nval=rsort[i].i; rval=rsort[i].r;
              seta(trgtSet, "n", nval);
              printf(" node:%d value:%lf dist:%lf\n", nval, val, sqrt(rval));
              sprintf(parameter[0],"%d", nval);
              sprintf(parameter[1],"%e", val);
              sprintf(parameter[2],"%e", sqrt(rval));
              write2stack(3, parameter);
            }  
          }
	}
        else
	{
          val=MAX_FLOAT;
          /* search the lowest value in range */
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(rsort[i].r>tol*tol) break;
            if(val>lcase[cur_lc].dat[cur_entity][rsort[i].i]) { val=lcase[cur_lc].dat[cur_entity][rsort[i].i]; nval=rsort[i].i; rval=rsort[i].r; }
          }
          if(nval)
	  {
            seta(trgtSet, "n", nval);
            printf(" node:%d value:%lf dist:%lf\n\n", nval, val, sqrt(rval));
            sprintf(parameter[0],"%d", nval);
            sprintf(parameter[1],"%e", val);
            sprintf(parameter[2],"%e", sqrt(rval));
            write2stack(3, parameter);
	  }
          else printf("\n found no node in range\n\n");
	}
      }
      break;
    }
    free(rsort);

    if((mode=='l')||(mode=='h'))
    {
      enq_nr++;
      sprintf(filenam,"enq_lc%d_e%d_%d.out", cur_lc+1, cur_entity+1, enq_nr);
      handle = fopen (filenam, "w+b");
      if (handle==NULL) { printf ("\nThe output file \"%s\" could not be opened.\n\n", filenam ); }
      else 
      {
        printf (" result is written to \"%s\"\n\n", filenam);
        fprintf(handle, " node: %d val: %f dist: %f\n", nval, val, sqrt(rval));
        fclose(handle);
      }
    }
  }

  if(set[setNr].anz_p)
  {
    /* calculate dr of all points and sort the indexes according to distance**2 (rsort[i].r) */ 
    if ( (rsort = (Rsort *)malloc( (set[setNr].anz_p+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 

    for(i=0; i<set[setNr].anz_p; i++)
    {
      n=set[setNr].pnt[i];
      if(dat[2][0]=='s')
      {
        if(set[setPos].anz_n)
        {
          dx=node[set[setPos].node[0]].nx-point[n].px;
          dy=node[set[setPos].node[0]].ny-point[n].py;
          dz=node[set[setPos].node[0]].nz-point[n].pz;
	}
        else if(set[setPos].anz_p)
        {
          dx=point[set[setPos].pnt[0]].px-point[n].px;
          dy=point[set[setPos].pnt[0]].py-point[n].py;
          dz=point[set[setPos].pnt[0]].pz-point[n].pz;
	}
        else return(0);
        rsort[i].r=dx*dx+dy*dy+dz*dz;
      }
      else if(dat[2][0]=='r')
      {
        if(ico[0]) dx=vco[0]-point[n].px; else dx=0.;
        if(ico[1]) dy=vco[1]-point[n].py; else dy=0.;
        if(ico[2]) dz=vco[2]-point[n].pz; else dz=0.;
        rsort[i].r=dx*dx+dy*dy+dz*dz;
      }
      else if(dat[2][0]=='c')
      {
        switch(dat[3][0])
        {
          case 'x':
          {
            nr=sqrt(point[n].pz*point[n].pz+point[n].py*point[n].py);
            nx=point[n].px;
          }
          break;
          case 'y':
          {
            nr=sqrt(point[n].pz*point[n].pz+point[n].px*point[n].px);
            nx=point[n].py;
          }
          break;
          case 'z':
          {
            nr=sqrt(point[n].py*point[n].py+point[n].px*point[n].px);
            nx=point[n].pz;
          }
          break;
        }
        if(ico[1]) dr=vco[1]-nr; else dr=0.;
        if(ico[2]) dx=vco[2]-nx; else dx=0.;
        rsort[i].r=dr*dr+dx*dx;
      }
      else
      {
        printf("parameter not recognized:%s\n", dat[2]);
        break;
      }
      rsort[i].i=n;
    }
    qsort( rsort, set[setNr].anz_p, sizeof(Rsort), (void *)compareRsort );

    switch(mode)
    {
      case 'i':
      {
        if(rsort[0].r<=tol*tol)
        { 
	  printf("p:%s r:%f\n",  point[rsort[0].i].name, rsort[0].r);
          seta(trgtSet, "p", rsort[0].i);
          sprintf(parameter[0],"%s", point[rsort[0].i].name);
          sprintf(parameter[1],"%e", rsort[0].r);
          write2stack(2, parameter);
	}
      }
      break;
      case 'a':
      {
        for (i=0; i<set[setNr].anz_p; i++)
        {
          if(rsort[i].r>tol*tol) break;
	  printf("p:%s r:%f\n",  point[rsort[i].i].name, rsort[i].r);
          seta(trgtSet, "p", rsort[i].i);
          sprintf(parameter[0],"%s", point[rsort[i].i].name);
          sprintf(parameter[1],"%e", rsort[i].r);
          write2stack(2, parameter);
        }
      }
      break;
    }
    free(rsort);
  }


  if(set[setNr].anz_l)
  {
    // calculate dr of all points and sort the indexes according to distance**2 (rsort[i].r)
    if ( (rsort = (Rsort *)malloc( (set[setNr].anz_l+1)*ddiv  * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 

    j=0;
    for(i=0; i<set[setNr].anz_l; i++)
    {
      l=set[setNr].line[i];
      for (n=0; n<line[l].nip; n+=3)
      {
        if(dat[2][0]=='s')
        {
          if(set[setPos].anz_n)
          {
            dx=node[set[setPos].node[0]].nx-((line[l].ip[n]* scale->w)+scale->x);
            dy=node[set[setPos].node[0]].ny-((line[l].ip[n+1]* scale->w)+scale->y);
            dz=node[set[setPos].node[0]].nz-((line[l].ip[n+2]* scale->w)+scale->z);
  	  }
          else if(set[setPos].anz_p)
          {
            dx=point[set[setPos].pnt[0]].px-((line[l].ip[n]* scale->w)+scale->x);  
            dy=point[set[setPos].pnt[0]].py-((line[l].ip[n+1]* scale->w)+scale->y);
            dz=point[set[setPos].pnt[0]].pz-((line[l].ip[n+2]* scale->w)+scale->z);
  	  }
          else return(0);
          rsort[j].r=dx*dx+dy*dy+dz*dz;
        }
        else if(dat[2][0]=='r')
        {
          if(ico[0]) dx=vco[0]-((line[l].ip[n]* scale->w)+scale->x); else dx=0.;
          if(ico[1]) dy=vco[1]-((line[l].ip[n+1]* scale->w)+scale->y); else dy=0.;
          if(ico[2]) dz=vco[2]-((line[l].ip[n+2]* scale->w)+scale->z); else dz=0.;
          rsort[j].r=dx*dx+dy*dy+dz*dz;
	  //printf("r:%lf %lf %lf %lf\n", rsort[j].r, dx,dy,dz);
        }
        else if(dat[2][0]=='c')
        {
          nx=(line[l].ip[n]* scale->w)+scale->x;
          ny=(line[l].ip[n+1]* scale->w)+scale->y;
          nz=(line[l].ip[n+2]* scale->w)+scale->z;
	  //printf("pnt ! %f %f %f\n",nx,ny,nz); 
          switch(dat[3][0])
          {
            case 'x':
            {
              nr=sqrt(ny*ny+nz*nz);
              //nx=nx;
            }
            break;
            case 'y':
            {
              nr=sqrt(nx*nx+nz*nz);
              nx=ny;
            }
            break;
            case 'z':
            {
              nr=sqrt(ny*ny+nx*nx);
              nx=nz;
            }
            break;
          }
          if(ico[1]) dr=vco[1]-nr; else dr=0.;
          if(ico[2]) dx=vco[2]-nx; else dx=0.;
          rsort[j].r=dr*dr+dx*dx;
        }
        else
        {
          printf("parameter not recognized:%s\n", dat[2]);
          break;
        }
        rsort[j].i=l;

        j++;
        if(j>=(set[setNr].anz_l+1)*ddiv  )
	{
          if ( (rsort = (Rsort *)realloc((Rsort *)rsort,  (j+1)  * sizeof(Rsort))) == NULL )
            printf("ERROR: realloc failed: Rsort\n\n" );
	} 
      }
    }
    if(j==0) goto nolines;

    qsort( rsort, j, sizeof(Rsort), (void *)compareRsort );

    switch(mode)
    {
      case 'i':
      {
        if(rsort[0].r<=tol*tol)
	{
          seta(trgtSet, "l", rsort[0].i);
          sprintf(parameter[0],"%s", line[rsort[0].i].name);
          sprintf(parameter[1],"%e", rsort[0].r);
          write2stack(2, parameter);
	}
      }
      break;
      case 'a':
      {
        for (i=0; i<j; i++)
        {
          if(rsort[i].r>tol*tol) break;
          seta(trgtSet, "l", rsort[i].i);
          sprintf(parameter[0],"%s", line[rsort[i].i].name);
          sprintf(parameter[1],"%e", rsort[i].r);
          write2stack(2, parameter);
        }
      }
      break;
    }
  nolines:;
    free(rsort);
  }


  if(set[setNr].anz_s)
  {
    // calculate dr of all points and sort the indexes according to distance**2 (rsort[i].r)
    if ( (rsort = (Rsort *)malloc( (set[setNr].anz_s+1)*ddiv*ddiv  * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 

    j=0;
    for(i=0; i<set[setNr].anz_s; i++)
    {
      l=set[setNr].surf[i];
      n=0;
      while((surf[l].npgn-n))
      {
        n++; // jump over the polygon token (ie.GL_POLYGON_TOKEN) 
        m=surf[l].pgn[n++];
        n+=3; // jump over the normal-vector
        for(k=0; k<m; k++)
        {
          //printf("%d %s %lf %lf %lf\n", k,surf[l].name, surf[l].pgn[n],surf[l].pgn[n+1],surf[l].pgn[n+2]); 
          if(dat[2][0]=='s')
          {
            if(set[setPos].anz_n)
            {
              dx=node[set[setPos].node[0]].nx-((surf[l].pgn[n]* scale->w)+scale->x);
              dy=node[set[setPos].node[0]].ny-((surf[l].pgn[n+1]* scale->w)+scale->y);
              dz=node[set[setPos].node[0]].nz-((surf[l].pgn[n+2]* scale->w)+scale->z);
    	    }
            else if(set[setPos].anz_p)
            {
              dx=point[set[setPos].pnt[0]].px-((surf[l].pgn[n]* scale->w)+scale->x);  
              dy=point[set[setPos].pnt[0]].py-((surf[l].pgn[n+1]* scale->w)+scale->y);
              dz=point[set[setPos].pnt[0]].pz-((surf[l].pgn[n+2]* scale->w)+scale->z);
    	    }
            else return(0);
            rsort[j].r=dx*dx+dy*dy+dz*dz;
          }
          else if(dat[2][0]=='r')
          {
            if(ico[0]) dx=vco[0]-((surf[l].pgn[n]* scale->w)+scale->x); else dx=0.;
            if(ico[1]) dy=vco[1]-((surf[l].pgn[n+1]* scale->w)+scale->y); else dy=0.;
            if(ico[2]) dz=vco[2]-((surf[l].pgn[n+2]* scale->w)+scale->z); else dz=0.;
            rsort[j].r=dx*dx+dy*dy+dz*dz;
  	    //printf("r:%lf %lf %lf %lf\n", rsort[j].r, dx,dy,dz);
          }
          else if(dat[2][0]=='c')
          {
            nx=(surf[l].pgn[n]* scale->w)+scale->x;
            ny=(surf[l].pgn[n+1]* scale->w)+scale->y;
            nz=(surf[l].pgn[n+2]* scale->w)+scale->z;
  	    //printf("pnt ! %f %f %f\n",nx,ny,nz); 
            switch(dat[3][0])
            {
              case 'x':
              {
                nr=sqrt(ny*ny+nz*nz);
                //nx=nx;
              }
              break;
              case 'y':
              {
                nr=sqrt(nx*nx+nz*nz);
                nx=ny;
              }
              break;
              case 'z':
              {
                nr=sqrt(ny*ny+nx*nx);
                nx=nz;
              }
              break;
            }
            if(ico[1]) dr=vco[1]-nr; else dr=0.;
            if(ico[2]) dx=vco[2]-nx; else dx=0.;
            rsort[j].r=dr*dr+dx*dx;
          }
          else
          {
            printf("parameter not recognized:%s\n", dat[2]);
            break;
          }
          rsort[j].i=l;
  
          j++;
          if(j>=(set[setNr].anz_s+1)*ddiv*ddiv  )
  	  {
            if ( (rsort = (Rsort *)realloc((Rsort *)rsort,  (j+1)  * sizeof(Rsort))) == NULL )
              printf("ERROR: realloc failed: Rsort\n\n" );
          } 

          n+=3; 
        }
      }
    }
    if(j==0) goto nosurfs;

    qsort( rsort, j, sizeof(Rsort), (void *)compareRsort );

    switch(mode)
    {
      case 'i':
      {
        if(rsort[0].r<=tol*tol)
	{
          seta(trgtSet, "s", rsort[0].i);
          sprintf(parameter[0],"%s", surf[rsort[0].i].name);
          sprintf(parameter[1],"%e", rsort[0].r);
          write2stack(2, parameter);
	}
      }
      break;
      case 'a':
      {
        for (i=0; i<j; i++)
        {
          if(rsort[i].r>tol*tol) break;
          seta(trgtSet, "s", rsort[i].i);
          sprintf(parameter[0],"%s", surf[rsort[i].i].name);
          sprintf(parameter[1],"%e", rsort[i].r);
          write2stack(2, parameter);
        }
      }
      break;
    }
  nosurfs:;
    free(rsort);
  }


  scalNodes ( anz->n, node, scale );
  scalPoints( anzGeo->p, point, scale);

  return(1);
}
