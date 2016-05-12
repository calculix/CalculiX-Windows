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
#include <time.h>
#include <sys/utsname.h>

#define     VERSION         "2.10.1"
#define     TEST            0

#define   GLUT_WEEL_UP 3
#define   GLUT_WEEL_DOWN 4

/* special cases: */
/* temporary conversion from old to new bias definition */
int OLD_BIAS_DEF=0;
/* generate sets from pressure-loads */
int MAKE_SETS_DEF=1;


void generalinfo()
{
  printf("  --------------------------------------------------------------------  \n");
  printf("                           CALCULIX                                     \n");
  printf("                    - GRAPHICAL INTERFACE -                             \n");
  printf("      Version %s                                                        \n", VERSION);
  printf("                                                                        \n");
  printf("                                                                        \n");
  printf("      A 3-dimensional pre- and post-processor for finite elements       \n");
  printf("               Copyright (C) 1996, 2002 Klaus Wittig                    \n");
  printf("                                                                        \n");
  printf("      This program is free software; you can redistribute it and/or     \n");
  printf("      modify it under the terms of the GNU General Public License as    \n");
  printf("      published by the Free Software Foundation; version 2 of           \n");
  printf("      the License.                                                      \n");
  printf("                                                                        \n");
  printf("      This program is distributed in the hope that it will be useful,   \n");
  printf("      but WITHOUT ANY WARRANTY; without even the implied warranty of    \n"); 
  printf("      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the      \n");
  printf("      GNU General Public License for more details.                      \n");
  printf("                                                                        \n");
  printf("      You should have received a copy of the GNU General Public License \n");
  printf("      along with this program; if not, write to the Free Software       \n");
  printf("      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.         \n");
  printf("  --------------------------------------------------------------------  \n");
  printf("\n usage: cgx [parameter] filename [ccxfile]                            \n\n");
  printf(" Parameters:                                                            \n");
  printf("  -a  auto-mode, geometry file derived from a cad-file must be provided \n");
  printf("  -b  build-mode, geometry (command) file must be provided              \n");
  printf("  -bg background, suppress creation of graphic output                   \n");
  printf("      otherwhise as -b, geometry (command) file must be provided        \n");
  printf("  -c  read an solver input file (ccx)                                   \n");
  printf("  -duns2d  read duns result files (2D)                                  \n");
  printf("  -duns3d  read duns result files (3D)                                  \n");
  printf("  -foam    read openFoam result files                                   \n");
  printf("  -isaac2d [-pref<val> -tref<val> -R<val>] read isaac result files (2D) \n");
  printf("  -isaac3d [-pref<val> -tref<val> -R<val>] read isaac result files (3D) \n");
  printf("  -f06  read Nastran f06 file                                           \n");
  printf("  -ng   read Netgen native format                                       \n");
  printf("  -step read a step file (only points and lines)                        \n");
  printf("  -stepsplit read step and write its single parts to the filesystem     \n");
  printf("  -stl  read stl triangles                                              \n");
  printf("  -v  (default) read a result file in frd-format and optional a solver  \n");
  printf("      input file (ccx) which provides the sets and loads used in the    \n");
  printf("      calculation.                                                      \n");
  printf("                                                                        \n"); 
  printf("  special purpose options:                                              \n");                                               
  printf("  -mksets       make node-sets from *DLOAD-values (setname:''_<value>'')\n");  
  printf("  -read         forces the program to read the complete result-file     \n"); 
  printf("                at startup                                              \n");     
  printf("                                                                        \n"); 
}

/* 
Necessary system-routines and libs:
    glut
    openGL, libGL and libGLU (the one from SGI which handles nurbs) 
    system
    sort
    rm
*/
/* 
Necessary stand alone programs:
- for postscript Hardcopys 
    convert
- for multi-picture postscript Hardcopys 
    convert, pstops (from psutils), ghostscript (new version (2015) replace -sDEVICE=pswrite with ps2write)
- for 2D plots
    gnuplot
- for online-help
    netscape, or other html-browser
*/
/* 
TODO:
- "big" node and element numbers should be possible, hash-table has to be implemented
- to be included in the menu:
  glPointSize ();
  glLineWidth ();
- trX e,s,c should be possible with the unstructured mesher
*/
/* 
POSSIBLE TROUBLE:
- remove the lnes:    //glShadeModel ( GL_SMOOTH ); when no problems have occured
    only in plotFunktions.c exists then  glShadeModel ( GL_SMOOTH ); in drawNurs_plot. It might also be deleted.
- if(flipflop) etc. was commented. might cause trouble on some systems with movi and hcpy
- some points and sets might use the same name. This leads to problems with lines. The line command 
  determines if it is a center-point or a sequence (seqa) based on the name of the track-parameter.
  If a point and a seqa use both the specified name then only a sraight line can be generated.

  In the moment the problem is dealed with in a way that the seqa-names start with an "A" and points start with a "D"
  But if the user has named this entities himself then a crash might still happen.

- seach NEWELEM: This block might be unnecessary. Has to be checked
*/
/* 
Known bugs:
- search for debug
*/


/* keyboard history */
char **key_history=(char **)NULL;
int nkey_history=0, key_pointer, curshft=0;
char  keystroke[MAX_LINE_LENGTH];


/* Display-lists */
GLuint list_model_edges, list_surf_edges, list_elem_edges ;
GLuint list_elem_light, list_elem_load, list_elem_elstress;           
GLuint list_surf_light, list_surf_load;
GLuint list_animate_light, *list_animate=NULL;
GLint  range_animate_light;
GLuint *list_animate_model_edges=NULL, *list_animate_surf_edges=NULL, *list_animate_elem_edges=NULL;

Summen    anz[1]; 
SumGeo    anzGeo[1];
SumAsci   sumAsci[1];

SpecialSet specialset[1];

Nodes     *node=NULL;
Datasets *lcase=NULL;
NodeBlocks *nBlock;
Alias     *alias=NULL;
Sets      *set=NULL;
Shapes    *shape=NULL;
Materials *material=NULL; 
Amplitudes *amplitude=NULL; 
Psets     *pset=NULL;
Values    *value=NULL;
Points    *point=NULL;
Lines     *line=NULL;
Lcmb      *lcmb=NULL;
Gsur      *surf=NULL;
Gbod      *body=NULL;
Nurbl     *nurbl=NULL;
Nurbs     *nurbs=NULL;
BGpicture *bgpicture;


Elements  *e_enqire=NULL;     /* elem-array indexed by elem-number */
double     *vp=NULL;
Scale     scale[1];
Faces     *face;
Edges     *edge=NULL;             /* model-edges           */
Texts     *ntext=NULL;             /* user texts           */

Meshp meshp={ALPHA,BETA,NADAPT};   /* mesh parameters for tr3u elements. Used in mesh2d */


#if INX_MODE
int       *colNr;
#endif
#if TEX_MODE
double     *colNr;
#endif
GLfloat   *contur_tex=NULL;

/* for CFD-meshing */
double pref=1.e5, tref=288., R_GAS=287.1;

struct utsname  cursys[1];
int             bitplanes;            /*  colorbuffer depth */

Display       *dpy;
int           dpycells;
Colormap      cmap;
XColor        *xcolor;
unsigned long *pixels_return;
unsigned int  npixels;
double         priv_cmap[256][3];


/* Main-Prog and GLUT-Window Management */
char  buffer[MAX_LINE_LENGTH];                          /* string buffer */
char  datin[MAX_LINE_LENGTH];                          /* Input-data-file */
char  browser[MAX_LINE_LENGTH]=BROWSER;  /* html-browser */
char  psviewer[MAX_LINE_LENGTH]=PSVIEWER;  /* postscript viewer */
char  helpfile[10][MAX_LINE_LENGTH]=HELPFILE;  /* help-file */
int   width_ini={INI_SCREEN}, height_ini={INI_SCREEN}; /* Grafik-Fensterbreite/hoehe */
int   width_menu={INI_MENU_WIDTH}, height_menu={INI_MENU_HEIGHT};
int   width_w0, height_w0;
int   width_w1, height_w1;
double   aspectRatio_w1=1.;         /* width_w1/height_w1 */
int   open_cgxs;              /* No. of cgx-sessions on that screen */
int   w0, w1, w2;                           /* window identifier  */
int   activWindow={-1};                     /* das aktuelle Fenster */
int   animList=0;                           /* die aktuelle Displayliste fuer animation */

int   basCol[3]={0,1,2};       /* color indexes due to basic colormap: 0=black 1=white 2=sliver (grey) */
int   foregrndcol=0, backgrndcol=1;          /* default fore- and background color */
double foregrndcol_rgb[4]={0.,0.,0.,1.};
double backgrndcol_rgb[4]={1.,1.,1.,1.};
char          entity_k[SET_COLS]={'k','w','n','r','g','b','y','m','t'};  /* predefined colors of entities */
GLfloat       entity_r[SET_COLS]={ 0., 1., .6, 1., 0., 0., 1., 1., 0. };
GLfloat       entity_g[SET_COLS]={ 0., 1., .6, .0, 1., 0., 1., 0., 1. };
GLfloat       entity_b[SET_COLS]={ 0., 1., .6, .0, 0., 1., 0., 1., 1. };

int   submenu_load=-1, submenu_view=-1, mainmenu=-1;        /* menu identifier */
int   submenu_scala=-1, submenu_animate=-1, submenu_cut=-1, submenu_graph=-1, submenu_help=-1;
int   submenu_orientation=-1, submenu_hardcopy=-1;
int   subsubmenu_entity=-1, subsubmenu_parameter=-1;
int   subsubmenu_animTune=-1, subsubmenu_animSteps=-1;
int   subsubmenu_animPeriod=-1;

GLfloat lmodel_twoside[] = { GL_TRUE };
GLfloat lmodel_oneside[] = { GL_FALSE };
double dx ,dy;                                      /* Mauskoordinaten im bereich +-1*/
int   xpixels ,ypixels;                            /* Mauskoordinaten in pixel, links unten 0,0 */
double beginx, beginy;                              /* letzter mousepoint */
double trackbsize={0.8};                            /* TRACKBALLSIZE */
double curquat[4];                                  /* Matrix aus Trackball */
double lastquat[4];                                 /* letzte Matrix aus Trackball*/
GLint   gl_max_eval_order=8;                         /* max order of NURBS */
int   MouseMode;                                   /* status maustasten */
GLdouble dx_cur={PICK_LENGTH}, dy_cur={PICK_LENGTH};   /* pick-cursor Area */
GLdouble R[4][4];                                   /* Rotationsmatrix */
GLdouble dR[4][4];                                  /* dR= R-Rmem fuer center */
GLdouble Rmem[4][4];
double v[4];                                        /* drehkorrekturen fuer centerPkt */
double vmem[4];                                     /* kor. bis auswahl eines neuen drehpkts */

char  inpformat=0;                     /* defines the start-up mode of cgx */
char  iniActionsFlag=0;                /* if set to 1 actions during idle will be performed */
char  frameFlag={1};                   /* mit (1) oder ohne Rahmen um das Grafikfenster */
char  filenamFlag={1};                 /* mit (1) oder ohne filename im Menufenster */
char  textFlag={1};                    /* mit (1) oder ohne text im Menufenster */
char  printFlag=0;                     /* printf on/off on=1 (kommando 'msg' 'on'|'off' )*/
char  scalaFlag={1};                   /* mit (1) oder ohne scala und wertetexte */ 
char  sequenceFlag=0;                  /* 1: play a sequence of LC */
char  vectorFlag=0;                    /* 0: scalar plot, 1: vector plot */
char  addDispFlag=0;                   /* 0: original node-coordinates, 1: node-coordinates+displacements */
char  flipColorFlag=0;                 /* 0: high values use red, low use blue in scale; 1: flipped */
char  graphFlag=0;                     /* 0:out, 1:graph line, 2: graph n, 3: graph t */
char  cutFlag=0;                       /* 0:out, 1: last node selected, cut structure */
char  v_dim=0;                         /* 1: scalar plot, 2: a 2D vector plot, 3: a 3D vectorplot, 4: a 3D vectorplot with signed vals */
char  lightFlag=0;                     /* sequence with illumination */
char  movieFlag=0;                     /* >0: save sequence of gif pictures */
int   movieFrames=0;                   /* if >0 number of frames until the movie is created. frame generation stops then. */
char  movieCommandFile[MAX_LINE_LENGTH];        /* stores the file name of a command file which will be executed after the movie is created (frames option only) */
int seqLC[4], seq_nlc=0;               /* selected ds for sequence (1. start, 2. defines step-with, 3. end, seq_nlc might be 1 to 3 */
DsSequence dsSequence;                 /* datasets (ds) for sequence of results */
int   lcase_animList={-1};                       /* additional lcase for the values of the animation */
int   read_mode=0;                               /* if 1 read data immediatelly, else read on demand */
int   step_mode=0;                               /* if 1 read step data and write the single parts from an assembly to the filesystem */
char  automode=0;                       /* set to 1 to perform various automatic actions as determining of divisions etc. during reading of command files */

char  animFlag={0};                    /* animation of the selected Dataset */
char  surfFlag={1};                     /* zeichne nur Oberflaechenelemente (1), sonst (0)*/
char  modelEdgeFlag={1};                /* zeichne mit Kanten (1), sonst (0)*/
char  elemEdgeFlag={0};                 /* zeichne mit Surface Kanten (1), sonst (0)*/
char  modelEdgeFlag_Static={0};         /* zeichne mit Kanten (1), sonst (0), stehende Kanten waerend animationen */
char  elemEdgeFlag_Static={0};          /* zeichne mit Surface Kanten (1), sonst (0)*, stehende Kanten waerend animationen */
char  drawMode={2};                     /* actual draw-function (Load=1, Light=2, 3 not used, Preprocessor=4, Vector=5)*/
char  stopFlag=0;                      /* stop/start animation */
char  zoomFlag={0};                    /* (1) zoom Modus */
char  centerFlag={0};                  /* (1) search centerPnt */
char  enquireFlag={0};                 /* (1) enquire node-values */
int   centerNode=0;                    /* Nr of center Node, 0:no centernode */
char  movezFlag={0};                     /* (1) move Model in Z Direction through the cuting plane */
char  pickfunc[MAX_LINE_LENGTH];       /* pick-function like "qenq" "qadd" "qrem" ..   */
char  pickFlag={0};                    /* 1 if picking is active */
char  delPntFlag={0};                  /* 1: deleted points exists */
char  delShapeFlag={0};                /* 1: deleted shapes exists */
char  delLineFlag={0};                 /* 1: deleted lines exists */
char  delLcmbFlag={0};                 /* 1: deleted lcmbs exists */
char  delSurfFlag={0};                 /* 1: deleted surfs exists */
char  delBodyFlag={0};                 /* 1: deleted bodys exists */
char  delNursFlag={0};                 /* 1: deleted Nurbs exists */
char  delSetFlag={0};                  /* 1: deleted sets exists */
char  fillSurfFlag={0};                /* 1: generate triangles for surface-rendering and projection */
int   hcpyFlag={0};                    /* triggers createHardcopy if !=0 */

char  mode[2]={'i'};                      /* pickmode */
double minvalue, maxvalue;                         /* Wertebereich */
int   steps={21};                                  /* Schrittweite der Farbscala */
int   offset, maxIndex;                            /* offset+steps-1 = maxIndex */
int   anim_steps={8};                              /* Animationen pro Schwingung */
double anim_faktor={1.};                           /* Scalierung der Amplitude */
int   *anim_alfa;                                  /* amplitude der Animation fuer Bildbeschriftung */
int   time_per_period={MILLISECONDS_PER_PERIOD}; /* fuer Animation */
int   frameNr={0};                                 /* zaehlt alle animierten Bilder */
int   halfperiod={0};                             /* 1:Animation nur der posit. Halbperiod */
int   hcpy={0};                                    /* hcpy=1 Hardcopy angefordert */
double dtx={0.}, dty={0.}, dtz={0.}, drx, dry, drz, ds={0.5};            /* Verschiebungen */
double centerPnt[3];                                /* Rotationszentrum */
int   cur_entity={0};                                       /* aktive entity (component), entity in menu= cur_entity+1*/
int   entity_v[6];                                         /* components of a vector-entity */
double v_factor;                                    /* scaling-factor for the vectors in the vector-plot */
double v_scale={1.};                                     /* additional scaling-factor for the vectors */
int       pre_lc={0};                                      /* pre-selected Dataset, active after entity is selected */
int       cur_lc={0};                                          /* aktive Dataset */
int       entity_buf=0;
void      *glut_font[]=GLUT_FONT;                      /* glut fonts */
int       pixPerChar[SUM_GLUT_FONTS]=GLUT_FONT_WIDTH;
int       legend_font=DEF_GLUT_FONT;                         /* active font for the legend */
int       draw_font=DEF_GLUT_FONT;                         /* active font for the annotation of entities */
int       menu_font=DEF_GLUT_FONT;                         /* active font for the menu */
int       elemMat[MAX_MATERIALS]={1,1};      /*  Material Numbers, Number of Materials stored in elemMat[0]  */
int       nasMpc=1;                                       /* 1: areampc generates mpcs; 0: rbes with optional heat-expansion-coefficient */
double    nasRbeHec=0.; 
char  picture_text[MAX_LINE_LENGTH]= {" "};               /* Text on window base line */

double     gtol={GTOL};                                    /* geometric tolerance for merging */
int     ddiv={DEF_LINE_DIV};
double     dbias=1;
int       neqn={0};            /* start-number of equations in ansys, MID in nastran */
int       setall=0;              /* setNr of the default set "all" */

/* nr of hardcopies */
int psNr=1, tgaNr=1, gifNr=1, pngNr=1;

/* track the open sets */
OpenSets openSets[1];

/* the copied node-sets which have to be filled with values from new loaded Datasets */
CopiedNodeSets copiedNodeSets[1];

/* element quality thresholds */
Eqal eqal={0.,0.,0.};

/* buffer to store related infos for the temporory qcut-nodes, used for data-interpolation */
Qcut_nodes *qcut_nod=NULL;

/* buffer to store values */
char **valuestack=NULL;
int valuestack_ptr=0, valuestackFlag=0;


int askGLError(char buffer[MAX_LINE_LENGTH])
{
  GLenum error = glGetError();
  if (error == (GLenum)GL_NO_ERROR) return(1);
  else if (error == (GLenum)GL_INVALID_ENUM) printf ("in:%s GL_INVALID_ENUM\n",buffer);
  else if (error == (GLenum)GL_INVALID_VALUE) printf ("in:%s GL_INVALID_VALUE \n",buffer);
  else if (error == (GLenum)GL_INVALID_OPERATION) printf("in:%s GL_INVALID_OPERATION\n",buffer);
  else if (error == (GLenum)GL_STACK_OVERFLOW) printf ("in:%s  GL_STACK_OVERFLOW\n",buffer);
  else if (error == (GLenum)GL_STACK_UNDERFLOW) printf ("in:%s  GL_STACK_UNDERFLOW\n",buffer);
  else if (error == (GLenum)GL_OUT_OF_MEMORY) printf ("in:%s  GL_OUT_OF_MEMORY\n",buffer);
  else {printf("glGetError detects unknown error %d in:%s\n", error, buffer); return (-1);}
  return (0);
}



/* realloc_colNr() must be executed before the call to nodalDataset() and each time new nodes were created */
void realloc_colNr(void)
{
  int i;
#if INX_MODE
  if ( (colNr = (int *)realloc((int *)colNr, (anz->nmax+1) * sizeof(int))) == NULL )
    printf("\n\n ERROR: realloc failed colNr\n\n") ;
  else
    for(i=0; i<=anz->nmax; i++) colNr[i]=0;
#endif
#if TEX_MODE
  if ( (colNr = (double *)realloc((double *)colNr, (anz->nmax+1) * sizeof(double))) == NULL )
    printf("\n\n ERROR: realloc failed colNr\n\n") ;
  else
    for(i=0; i<=anz->nmax; i++) colNr[i]=0.;
#endif
}



/* the node pointer must not be changed inside the function. Since that is the case the *node is changed to *node_dummy
   and the global *node is used which is always correct so far */
/* realloc_colNr() must be executed before the call to his function and each time new nodes were created */
#if INX_MODE
void nodalDataset( int entity, int lc, Summen *anz, Scale *scale, Nodes *node_dummy
                  , Datasets *lcase, int *colNr, int offset, int maxIndex, int steps, int scalaFlag )
#endif 
#if TEX_MODE
void nodalDataset( int entity, int lc, Summen *anz, Scale *scale, Nodes *node_dummy
                  , Datasets *lcase, double *colNr, int scalaFlag )
#endif 
{
  register int i,j, n;
  int n1, n2, settmp, allNodesFlag;
  int nmax=0, nmin=0;
  double vmax, vmin;                            /* max,min Werte der values */
  char parameter[20][MAX_LINE_LENGTH];

  if(!anz->l)
  {
      printf(" WARNING: No values available (should not come to this point)\n");
      return;    
  }

  /* check if the data of the specified lcase (Dataset) are already available */
  if (!lcase[lc].loaded)
  {
    if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
    {
      printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
      return;
    }
    calcDatasets( lc, anz, node, lcase );
    recompileEntitiesInMenu(lc);
  }

  /* if currently a section (qcut) is in use realloc the lcase and generate the necessary values */
  settmp=getSetNr("-qcut");
  if(settmp>-1) updLcase(lc, settmp); 
  else
  {
    if ( (lcase[lc].dat[entity] = (float *)realloc(lcase[lc].dat[entity], (anz->nmax+1) * sizeof(float))) == NULL )
      {  printf("\n\n ERROR: realloc failure updLcase\n\n" ); return; }
  }

  /* check if the specified lcase-component is allocated */
  if (entity>=lcase[lc].ncomps )
  {
    errMsg("ERROR: Component not available\n");
    return;
  }

  /* remove the displacements on node-coords if the time-step has changed */
  // TBD: do only if (lcase[selection].step_number!=lcase[cur_lc].step_number)
  if(addDispFlag==1)
  {
    printf("\n displacements will be updated\n");
    addDispToCoordinates(node);
    addDispToCoordinates(node);
  }

  for (i=0; i<anz->n; i++ )
  {
    if(node[node[i].nr].pflag==0) continue;
    lcase[lc].dat[entity][node[i].nr] = 9999999999;
  }

  for ( i=0; i<anz->e; i++ )
  {
    switch(e_enqire[e_enqire[i].nr].type)
    {
      case 4:
      for (n=0; n<3; n++)  /* create new vals at nodes in center of areas */
      {
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[20+n]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[1+n]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[5+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[4+n]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[8+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[13+n]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[16+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[12+n]]) ;
      }
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[23]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[4]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[7]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[11]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[12]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[19]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[15]]) ;
      for (n=0; n<2; n++)  
      {
        n1=n*4;
        n2=n*8;
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[24+n]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0+n1]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[1+n1]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[2+n1]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3+n1]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[8+n2]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[9+n2]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[10+n2]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[11+n2]]) ;
      }
      break;


      case 5:
      for (n=0; n<2; n++) 
      {
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[15+n]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[1+n]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[4+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3+n]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[6+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[10+n]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[12+n]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 9+n]]) ;
      }
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[17]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[2]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[5]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 8]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 9]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[14]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[11]]) ;
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[18]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[2]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[1]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 8]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 7]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 6]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 0]]) ;
        lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[19]] = -0.25* (
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[4]]    +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[5]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3]] )  + 0.5*(
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[12]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[13]]   +
          lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[14]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[ 3]]) ;
      break;


      case 10:
      lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[8]] = -0.25* (
      lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[0]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[1]] +
      lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[3]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[2]])+0.5*(
      lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[4]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[6]] +
      lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[7]]+lcase[lc].dat[entity][e_enqire[e_enqire[i].nr].nod[5]]);
      break;
    }
  }

  /* ------------  Darstellung scalieren -------------------  */

  if(scalaFlag)
  {
    /* scan all nodes of all entities which are used on the display */

    allNodesFlag=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if ((set[pset[j].nr].name[0] == 'a')&&(set[pset[j].nr].name[1] == 'l')&&(set[pset[j].nr].name[2] == 'l')&&(set[pset[j].nr].name[3] == 0))
      {
        allNodesFlag=1;
        break;
      }
    }

    if((allNodesFlag)||(anzGeo->psets==0))
    {
      vmax=lcase[lc].max[entity];
      vmin=lcase[lc].min[entity];
      nmax=lcase[lc].nmax[entity];
      nmin=lcase[lc].nmin[entity];
    }
    else
    {
      /* adjust the scala */
      vmin=MAX_FLOAT;
      vmax=-MAX_FLOAT;

      delSet(specialset->tmp);
      if( (settmp=pre_seta( specialset->tmp, "i", 0 )) <0 ) return;
      for (j=0; j<anzGeo->psets; j++ )
      {
        if (pset[j].type[0]=='f') for(i=0; i<set[pset[j].nr].anz_f; i++) seta( settmp, "f", set[pset[j].nr].face[i] );
        if (pset[j].type[0]=='e') for(i=0; i<set[pset[j].nr].anz_e; i++) seta( settmp, "e", set[pset[j].nr].elem[i] );
        if (pset[j].type[0]=='n') for(i=0; i<set[pset[j].nr].anz_n; i++) seta( settmp, "n", set[pset[j].nr].node[i] );
      }
      completeSet( specialset->tmp, "do");
      for (i=0; i<set[settmp].anz_n; i++ )
      {
        if(lcase[lc].dat[entity][set[settmp].node[i]] < vmin ) { vmin = lcase[lc].dat[entity][set[settmp].node[i]]; nmin=set[settmp].node[i]; }
        if(lcase[lc].dat[entity][set[settmp].node[i]] > vmax ) { vmax = lcase[lc].dat[entity][set[settmp].node[i]]; nmax=set[settmp].node[i]; }
      }
  
      /* if no plotset is defined */
      if(vmin==MAX_FLOAT)
      {
        vmax=lcase[lc].max[entity];
        vmin=lcase[lc].min[entity];
        nmin=lcase[lc].nmin[entity];
        nmax=lcase[lc].nmax[entity];
      }
    }
    if(scale->smin==scale->smax)
    {
      scale->smin=vmin;
      scale->smax=vmax;
    }

    printf ("\n%d %s %f %s %s %s\n", lc+1, lcase[lc].dataset_name, lcase[lc].value, lcase[lc].dataset_text, lcase[lc].name, lcase[lc].compName[entity] );
    printf (" Extremal values from displayed mesh-entities:\n max:%e at node:%d\n min:%e at node:%d\n", vmax, nmax, vmin, nmin);
    sprintf(parameter[0],"%d", lc+1);
    sprintf(parameter[1],"%s", lcase[lc].dataset_name);
    sprintf(parameter[2],"%e", lcase[lc].value);
    sprintf(parameter[3],"%s", lcase[lc].dataset_text);
    sprintf(parameter[4],"%s", lcase[lc].name);
    sprintf(parameter[5],"%s", lcase[lc].compName[entity]);
    sprintf(parameter[6],"%e", vmax);
    sprintf(parameter[7],"%d", nmax);
    sprintf(parameter[8],"%e", vmin);
    sprintf(parameter[9],"%d", nmin);
    write2stack(10, parameter);
  }

  for (i=0; i<anz->n; i++ )
  {
    if(node[node[i].nr].pflag==-1) continue;
#if INX_MODE
    /* ColorIndex bestimmen colNr = (0->1)*(steps) + Offset */
    if ( lcase[lc].dat[entity][node[i].nr] <= scale->smin )
    {
      colNr[node[i].nr] = offset;
    }
    else if ( lcase[lc].dat[entity][node[i].nr] >= scale->smax )
    {
      colNr[node[i].nr] = maxIndex;
    }
    else
    {
      colNr[node[i].nr] = (lcase[lc].dat[entity][node[i].nr]-scale->smin)/(scale->smax-scale->smin)*steps  +offset;
      if ( colNr[node[i].nr] > maxIndex )  colNr[node[i].nr] = maxIndex;
      if ( colNr[node[i].nr] < offset )  colNr[node[i].nr] = offset;
    }
#endif
#if TEX_MODE
    //printf("i:%d n:%d e:%d lc:%d smin:%f v:%f\n", i,node[i].nr,entity,lc,scale->smin,lcase[lc].dat[entity][node[i].nr]);
    if ( lcase[lc].dat[entity][node[i].nr] <= scale->smin )
    {
      colNr[node[i].nr] = 0.;
    }
    else if ( lcase[lc].dat[entity][node[i].nr] >= scale->smax )
    {
      colNr[node[i].nr] = (double)steps/(double)TEX_PIXELS;
    }
    else
    {
      colNr[node[i].nr] = (lcase[lc].dat[entity][node[i].nr]-scale->smin)/(scale->smax-scale->smin) *(double)steps/(double)TEX_PIXELS;
    }
#endif
  }
}



void elementDataset( int entity, int lc, Summen *anz, Scale *scale, Datasets *lcase, int offset, int maxIndex, int steps )
{
  int   i;
  double **vp;
  //double vmax, vmin;                            /* max,min Werte der values */
  //int   nmax, nmin;                            /* nodes der max/min-Werte  */

  if(!anz->l)
  {
      printf(" WARNING: No values available\n");
      return;    
  }

  if ( (vp = (double **)malloc( (anz->emax+1) * sizeof(double))) == NULL )
    printf("\n\n ERROR: malloc failed vp\n");
  for (i=0; i<(anz->emax+1); i++)
    if ( (vp[i] = (double *)malloc( (2) * sizeof(double))) == NULL )
      printf("\n\n ERROR: malloc failed vp[%d]\n", i);

  /* check if the specified lcase-component is allocated */

/*
  if (lcase[lc].edat[entity] == NULL )
    errMsg("ERROR: Component not available\n");
  else 
  {
    for (i=0; i<num_etype[11]; i++ )
    {
      for (j=0; j<2; j++ )
      {
        vp[i][j]  = lcase[lc].edat[entity][cbeam[i].elem_nr][j];
      }
    }
  }
  vmax=lcase[lc].max[entity];
  vmin=lcase[lc].min[entity];
  printf ("\nDataset:%d name= %s", lc+1, lcase[lc].name);
  printf (" entity:%s\n", lcase[lc].compName[entity] );
  printf (" maxvalue:%e\n minvalue:%e \n", vmax, vmin);

  for (i=0; i<num_etype[11]; i++ )
  {
    for (j=0; j<2; j++ )
    {
      if (scale->smin==0)
      {
        scale->smin = vmin;
      }
      if (scale->smax==0)
      {
        scale->smax = vmax;
      }
#if INX_MODE
      if ( vp[i][j] <= scale->smin )
      {
        cbeam[i].ncol[j][0] = offset;
      }
      if ( vp[i][j] >= scale->smax )
      {
        cbeam[i].ncol[j][0] = maxIndex;
      }
      if ( (vp[i][j] > scale->smin) && (vp[i][j] < scale->smax) )
      {
        cbeam[i].ncol[j][0] = (vp[i][j]-scale->smin)/(scale->smax-scale->smin)*steps  +offset;
        if ( cbeam[i].ncol[j][0] > maxIndex )  cbeam[i].ncol[j][0]= maxIndex;
        if ( cbeam[i].ncol[j][0] < offset )  cbeam[i].ncol[j][0]= offset;
      }
      printf ("v:%lf col:%lf  \n", vp[i][j], cbeam[i].ncol[j][0] );
#endif
#if TEX_MODE
      if ( vp[i][j] <= scale->smin )
      {
        cbeam[i].ncol[j][0] = 0.;
      }
      if ( vp[i][j] >= scale->smax )
      {
        cbeam[i].ncol[j][0] = 1.;
      }
      if ( (vp[i][j] > scale->smin) && (vp[i][j] < scale->smax) )
      {
        cbeam[i].ncol[j][0] = (vp[i][j]-scale->smin)/(scale->smax-scale->smin);
      }
      printf ("v:%lf col:%lf  \n", vp[i][j], cbeam[i].ncol[j][0] );
#endif
    }
  }
*/

  for (i=0; i<(anz->emax+1); i++) if (vp[i]) free(vp[i]);
  if (vp) free(vp);
}



/* from j. baylor for tga-screen-shot */
int WriteTGA(char *filename, 
             short int width, 
             short int height, 
             char *imageData) {

   char cGarbage = 0;
   char pixelDepth = 32;
   char type = 2; // type = 2 for pixelDepth = 32 | 24, type = 3 for greyscale
   char mode = 4; // mode = pixelDepth / 8
   char aux;
   short int iGarbage = 0;
   FILE *file;
   int i;

   // open file and check for errors
   file = fopen(filename, "wb");
   if (file == NULL) return(-1);

   // write the header
   fwrite(&cGarbage, sizeof(char), 1, file);
   fwrite(&cGarbage, sizeof(char), 1, file);
   fwrite(&type, sizeof(char), 1, file);
   fwrite(&iGarbage, sizeof(short int), 1, file);
   fwrite(&iGarbage, sizeof(short int), 1, file);
   fwrite(&cGarbage, sizeof(char), 1, file);
   fwrite(&iGarbage, sizeof(short int), 1, file);
   fwrite(&iGarbage, sizeof(short int), 1, file);
   fwrite(&width, sizeof(short int), 1, file);
   fwrite(&height, sizeof(short int), 1, file);
   fwrite(&pixelDepth, sizeof(char), 1, file);
   fwrite(&cGarbage, sizeof(char), 1, file);

   // convert the image data from RGB(a) to BGR(A)
   if (mode >= 3)
   for (i=0; i < width * height * mode ; i+= mode) {
      aux = imageData[i];
      imageData[i] = imageData[i+2];
      imageData[i+2] = aux;
   }

   // save the image data
   fwrite(imageData, sizeof(char), width * height * mode, file);
   fclose(file);

   return(0);
}



/* This will save a screen shot to a file. */
void SaveTGAScreenShot(char *filename, int w, int h)
{
   char *imageData;
   imageData = (char *)malloc(sizeof(char) * w * h * 4);
   glReadPixels(0, 0, w, h,GL_RGBA,GL_UNSIGNED_BYTE, (GLvoid *)imageData);
   WriteTGA(filename,w,h,imageData);
   // release the memory
   free(imageData);
}



void getTGAScreenShot(int nr)
{
    char buffer[MAX_LINE_LENGTH];

    if(!inpformat) return;

    glutSetWindow(w0);
    SaveTGAScreenShot("0__.tga", glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    glutSetWindow(w1);
    SaveTGAScreenShot("1__.tga", glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    glutSetWindow(w2);
    SaveTGAScreenShot("2__.tga", glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    glutSetWindow(activWindow);
    sprintf( buffer, "composite -compose atop -gravity SouthWest -geometry +1+1 2__.tga 1__.tga 3__.tga");
    system (buffer);
    sprintf( buffer, "composite -compose atop -gravity NorthWest -geometry +%d+%d 3__.tga 0__.tga hcpy_%d.tga",
		(GLint)width_menu*19/20, (GLint)height_menu/10, nr);
    system (buffer);
    sprintf( buffer, "rm -f *__.tga %s",DEV_NULL);
    system (buffer);
}
/* end tga-screen-shot */



void createHardcopy( int selection )
{
  char buffer[MAX_LINE_LENGTH];

  if(!inpformat) return;

  if(selection==0)
  {
    /* generate movie from single gif files */
    sprintf( buffer, "make 1. %lf",(double)gifNr-1);
    pre_movie(buffer);
    sprintf( buffer, "clean");
    pre_movie(buffer);
    gifNr=1;
  }
  else
  {
    /* Hardcopy         */
    if(selection==1)
    {
      printf("create hcpy_%d.ps\n ",psNr);
      getTGAScreenShot(psNr);
      /* on some systems PS has to be changed to PS2 */
      //sprintf( buffer, "convert -density %dx%d -page +49+196 -gamma %lf hcpy_%d.tga PS:hcpy_%d.ps   ", (int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)),(int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)) , GAMMA, psNr, psNr);
      sprintf( buffer, "convert hcpy_%d.tga -page A4 hcpy_%d.ps",  psNr, psNr);
      system (buffer);
      printf("%s\n", buffer);
      sprintf( buffer, "rm -f hcpy_%d.tga %s",psNr,DEV_NULL);
      system (buffer);
      psNr++;
      printf ("ready\n");
    }
    if(selection==2)
    {
      printf("create hcpy_%d.tga\n ",tgaNr);
      getTGAScreenShot(tgaNr);
      tgaNr++;
      printf ("ready\n");
    }
    if(selection==3)
    {
      /* movie gif files */
      getTGAScreenShot(0);
      sprintf( buffer, "convert hcpy_0.tga _%d.gif",gifNr);
      printf("%s\n",buffer);
      system (buffer);
      gifNr++;
      if((movieFrames)&&(gifNr>movieFrames))
      {
        animList=0;
        movieFrames=0;
        movieFlag=0;
        sprintf( buffer, "rm -f  hcpy_0.tga %s", DEV_NULL2);
        system (buffer);
        createHardcopy(0);
        if(strlen(movieCommandFile))
        {
          pre_read(movieCommandFile); 
        }
      }
    }
    if(selection==4)
    {
      printf("create hcpy_%d.gif\n ",gifNr);
      getTGAScreenShot(gifNr);
      sprintf( buffer, "convert hcpy_%d.tga hcpy_%d.gif",gifNr,gifNr);
      system (buffer);
      sprintf( buffer, "rm -f hcpy_%d.tga %s",gifNr,DEV_NULL);
      system (buffer);
      gifNr++; 
      printf ("ready\n");
    }
    if(selection==5)
    {
      printf("create hcpy_%d.png\n ",pngNr);
      getTGAScreenShot(pngNr);
      sprintf( buffer, "convert hcpy_%d.tga hcpy_%d.png",pngNr,pngNr);
      system (buffer);
      sprintf( buffer, "rm -f hcpy_%d.tga %s",pngNr,DEV_NULL);
      system (buffer);
      pngNr++;
      printf ("ready\n");
    }
  }
}



void entryfunktion( int state )
{
  if (state==GLUT_ENTERED)
  {
    activWindow=w1;
    glutSetWindow(activWindow );
    glutPostRedisplay();
    glutSetWindow( w2);
    glutPostRedisplay();
  }
  else
  {
    activWindow=w0;
    glutSetWindow(activWindow );
    glutPostRedisplay();
  }
}



void WindowState( int state )
{
  if (state==GLUT_NOT_VISIBLE)
    activWindow=w0;
}



void Mouse( int x, int y )
{
  if (glutGet(GLUT_WINDOW_HEIGHT)==height_w1) activWindow=w1;
  else return;

  xpixels=x; ypixels=y-height_w1;
  dx= (width_w1/2.-(double)x)/width_w1*-2.  *aspectRatio_w1;
  dy= (height_w1/2.-(double)y)/height_w1*2.;
  //printf("xy:%d %d dxy:%f %f\n", x,y, dx,dy);

  if (MouseMode == 3)
  {
    trackball( 0, trackbsize, curquat, beginx, beginy, dx, dy);
    add_quats(curquat, lastquat, lastquat);
    build_rotmatrix( R, lastquat );
  }
  if (MouseMode == 1)
  {
    dtx+= (dx-beginx);
    dty+= (dy-beginy);
  }
  if (MouseMode == 2)
  {
    if (movezFlag)
    {
      dtz+= (dy-beginy);
    }
    else
    {
    dtx*=ds;
    dty*=ds;
    ds-= (beginy-dy)*ds;
    dtx/=ds;
    dty/=ds;
    }
  }
  beginx = dx;
  beginy = dy;

  if ( activWindow == w1 )
  {
    glutPostRedisplay();
    glutSetWindow( w2);
    glutPostRedisplay();
    glutSetWindow(activWindow );
  }
}



void qzoom()
{
  static int i=0;
  static double xcur[2], ycur[2];

  xcur[i]=dx; ycur[i]=dy;
  if (i)
  {
    i=0;
    zoom(xcur[0], ycur[0], xcur[1], ycur[1]);
  }
  else i=1;
  printf ("P%d\n", i);
}



void qcenter(int x, int y)
{
  int i;
  double puf[3];

  for (i=0;i<3;i++) puf[i]=centerPnt[i];
  centerFlag=!centerFlag;
  printf ("pic new center\n");

  mode[0]='i';
  strcpy( pickfunc, "qcnt");
  glutSetWindow( w1);
  dx_cur=PICK_LENGTH*2; dy_cur=PICK_LENGTH*2;
  pick( (char)'n', x, y );
  if( (puf[0]==centerPnt[0])&&(puf[1]==centerPnt[1])&&(puf[2]==centerPnt[2]) )
  {
    pick( (char)'p', x, y );
    if( (puf[0]==centerPnt[0])&&(puf[1]==centerPnt[1])&&(puf[2]==centerPnt[2]) )
    {
      printf (" found no node/point, reset center location\n");
      centerNode=0;
      centerPnt[0]=centerPnt[1]=centerPnt[2]=0;
      center( centerPnt[0], centerPnt[1], centerPnt[2]);
    }
  }
  dx_cur=PICK_LENGTH; dy_cur=PICK_LENGTH;
  pick( (char)'q', x, y );
  glutSetWindow( activWindow );
}



void qcutNodes(int x, int y)
{
  int i;
  int anz_n;
  extern int setNrbuf;
  static int cutnode[3]={0,0,0};

  setNrbuf=pre_seta( specialset->plot2d, "i", 0);
  set[setNrbuf].type=1;

  //pickFlag=1;
  mode[0]='i';
  strcpy( pickfunc, "qadd");
  glutSetWindow( w1);

  anz_n=set[setNrbuf].anz_n;
  printf ("pic node\n");

  dx_cur=PICK_LENGTH*2; dy_cur=PICK_LENGTH*2;
  pick( (char)'n', x, y );
  dx_cur=PICK_LENGTH; dy_cur=PICK_LENGTH;

  //printf("anz_n:%d set[setNrbuf].anz_n:%d\n", anz_n, set[setNrbuf].anz_n);
  //for(i=0; i<set[setNrbuf].anz_n; i++)  printf("n:%d\n",set[setNrbuf].node[i]); 
  if( anz_n==set[setNrbuf].anz_n )
  {
    printf (" found no node, try again\n");
  }
  else
  {
    if(cutFlag==4) { pre_cut( set[setNrbuf].node[set[setNrbuf].anz_n-1], 'v' ); setr(setNrbuf, "n", set[setNrbuf].node[set[setNrbuf].anz_n-1]); }
    else cutnode[cutFlag-1]=set[setNrbuf].node[set[setNrbuf].anz_n-1];
    if(cutFlag==3) for (i=0; i<3; i++) pre_cut( cutnode[i], 'n' );
    cutFlag=0; 
  }
  pick( (char)'q', x, y );
  glutSetWindow( activWindow );
}


void qsequence(int x, int y)
{
  int anz_n;
  extern int setNrbuf;

  setNrbuf=pre_seta( specialset->plot2d, "i", 0);
  set[setNrbuf].type=1;

  //pickFlag=1;
  mode[0]='i';
  strcpy( pickfunc, "qadd");
  glutSetWindow( w1);

  anz_n=set[setNrbuf].anz_n;
  printf ("pic node\n");

  dx_cur=PICK_LENGTH*2; dy_cur=PICK_LENGTH*2;
  pick( (char)'n', x, y );
  dx_cur=PICK_LENGTH; dy_cur=PICK_LENGTH;
  if( anz_n==set[setNrbuf].anz_n )
  {
    printf (" found no node, try again\n");
  }
  //pick( (char)'q', x, y );
  glutSetWindow( activWindow );
}



void qgraph(int x, int y)
{
  int i,j,n;
  char buffer[MAX_LINE_LENGTH];

  /* look if the selection was done for a 2D-plot (triggered in selectGraphMode) */

  pick( (char)'q', x, y );
  printf("\n Sequence %s created. This sequence will be kept for further use (see: prnt sq, graph)\n", specialset->plot2d);
  sprintf(buffer,"%s", specialset->plot2d);
  i=getSetNr(buffer);
  if((i<0)||set[i].anz_n==0)
  {
    j=getSetNr(specialset->copy);
    if(j>-1)
    {
      i=pre_seta(specialset->plot2d, "i", 0);
      for(n=0; n<set[j].anz_n; n++) seta(i,"n",set[j].node[n]);
    }
  }
  if((i>-1)&&set[i].anz_n>0)
  {
    if(graphFlag==1) sprintf(buffer,"%s l", specialset->plot2d);
    if(graphFlag==2) sprintf(buffer,"%s n", specialset->plot2d);
    if(graphFlag==3) sprintf(buffer,"%s t", specialset->plot2d);
    graph(buffer);
  }
  graphFlag=0; 
}



void MouseState( int button, int state, int x, int y )
{
  static int  weelWasUsedFlag=0;

  xpixels=x; ypixels=y-height_w1;
  dx= (width_w1/2.-(double)x)/width_w1*-2.  *aspectRatio_w1;
  dy= (height_w1/2.-(double)y)/height_w1*2.;
  beginx = dx;
  beginy = dy;
  MouseMode = 0;

  if (glutGet(GLUT_WINDOW_HEIGHT)==height_w1) activWindow=w1;
  else activWindow=w0;

  //printf("button:%d state:%d %d %d\n", button,state,x,y);
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    MouseMode = 3;
    if (zoomFlag) qzoom();
    if (centerFlag) qcenter(x, y);
    if (graphFlag) qsequence(x, y);
    if (cutFlag) qcutNodes(x, y);
    if (enquireFlag)
    {
      strcpy( pickfunc, "qenq");
      glutSetWindow( w1);
      printf ("pic node\n");
      dx_cur=PICK_LENGTH*2; dy_cur=PICK_LENGTH*2;
      mode[0]='i';
      pick( (char)'n', x, y );
      dx_cur=PICK_LENGTH; dy_cur=PICK_LENGTH;
      glutSetWindow( activWindow );
      pick( (char)'q', x, y );
      enquireFlag=0 ;
    }
  }
  else
  if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
  { MouseMode = 2; }
  else
  if (button == GLUT_WEEL_UP )
  {
    weelWasUsedFlag=1;
    if (movezFlag)
    {
      dtz+= (0.05);
    }
    else
    {
    dtx*=ds;
    dty*=ds;
    ds+= (0.05)*ds;
    dtx/=ds;
    dty/=ds;
    }
  }
  else
  if (button == GLUT_WEEL_DOWN )
  {
    weelWasUsedFlag=1;
    if (movezFlag)
    {
      dtz-= (0.05);
    }
    else
    {
    dtx*=ds;
    dty*=ds;
    ds-= (0.05)*ds;
    dtx/=ds;
    dty/=ds;
    }
  }
  else
  if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
  {
    MouseMode = 1;
    if (graphFlag) qgraph(x, y);
  }

  /* update the elem-edges in case the view was zoomed */
  if( (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)|| 
      ( ((button == GLUT_RIGHT_BUTTON && state == GLUT_UP)||(button == GLUT_LEFT_BUTTON && state == GLUT_UP))&&(weelWasUsedFlag) ))
  {
    weelWasUsedFlag=0;
    /* printf("GLUT_MIDDLE_BUTTON && state == GLUT_UP\n"); */
    if (elemEdgeFlag)
    {
      if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
      else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );
    }
  }

  if(( MouseMode==1 )&&( activWindow==w0 )) 
  {
    stopFlag=1;
    glutSetWindow( w1);
    glutPostRedisplay();
    glutSetWindow( activWindow );
    if((movieFlag>0)&&(stopFlag))
    {
      movieFrames=0;
      movieFlag=0;
      printf("movie stopped, please wait for ready (might take a while)\n");
      sprintf( buffer, "rm -f hcpy_0.tga %s",DEV_NULL);
      system (buffer);
      createHardcopy(0);
      printf("movie.gif ready\n");
    }
    else
    {
      animList++;
      if ((sequenceFlag)&&( animList>=dsSequence.nds)) animList=0;
      if ((!sequenceFlag)&&( animList>=anim_steps)) animList=0;
    }
  }
  if(( MouseMode==2 )&&( activWindow==w0 )) 
  {
    stopFlag=!stopFlag;
    if(animList>0) animList--;
    else
    {
      if (sequenceFlag) animList=dsSequence.nds-1;
      if (!sequenceFlag) animList=anim_steps-1;
    }
  }
  if ( activWindow == w1 )
  {
    glutPostRedisplay();
    glutSetWindow( w2);
    glutPostRedisplay();
    glutSetWindow(activWindow );
  }
}



void frame(void)
{
  int i;

  if(!inpformat) return;

  /* nodes in definierten wertebereich scalieren wg. beleuchtung!  */
  descalNodes( anz->n, node, scale);
  descalPoints( anzGeo->p, point, scale);
  descalSurfs( anzGeo->s, surf, scale);
  getScaleValues( setall, set, point, node, scale);
  scalPoints ( anzGeo->p, point, scale );
  scalNodes ( anz->n, node, scale );
  scalSurfs( anzGeo->s, surf, scale);
  dtx=0.; dty=0.; dtz=0.; ds=0.5;
  for (i=0; i<4; i++) vmem[i]=0; /* reset all kompensations (center()) */
  /* recalculate the line-shapes */
  for (i=0; i<anzGeo->l; i++) repLine(i);
  for (i=0; i<anzGeo->nurl; i++) repNurl(i);
  for (i=0; i<anzGeo->nurs; i++) repNurs(i);
  redraw();
}



void menu( int selection )
{
  switch (selection) {
  case 1:
    frame();
    break;
  case 2:
    zoomFlag=!zoomFlag;
    break;
  case 3:
    centerFlag=!centerFlag;
    pre_view("elem");
    break;
  case 4:
    enquireFlag=!enquireFlag;
    pre_view("elem");
    break;
  case 5:
    exit(0);
    break;
  }
}



void orientModel( int selection )
{
  switch (selection) {
  case 1:
    /* Trackballfunktion inizialisieren, Blickrichtung +x (rot_x)*/
    rot_x(1);
    break;
  case 2:
    /* Trackballfunktion inizialisieren, Blickrichtung -x (rot_x)*/
    rot_x(-1);
    break;
  case 3:
    /* Trackballfunktion inizialisieren, Blickrichtung +y (rot_y)*/
    rot_y(1);
    break;
  case 4:
    /* Trackballfunktion inizialisieren, Blickrichtung -y (rot_y)*/
    rot_y(1);
    rot_r(90.);
    rot_r(90.);
    break;
  case 5:
    /* Trackballfunktion inizialisieren, Blickrichtung +z (rot_z)*/
    rot_z(1);
    rot_c(-90);
    break;
  case 6:
    /* Trackballfunktion inizialisieren, Blickrichtung -z (rot_z)*/
    rot_z(-1);
    rot_c(90);
    break;
  }
}



void markHardcopy( int selection )
{
  if(!inpformat) return;

  if(selection==3)
  {
    movieFlag=1;
    printf(" start recording movie\n");
    printf("   stop recording with right mouse key while in menu area of the window\n");
    sprintf( buffer, "rm -f _*.gif %s",DEV_NULL);
    system (buffer);
    sprintf( buffer, "rm -f movie.gif %s",DEV_NULL);
    system (buffer);
    stopFlag=0;
  }
  else
  {
    hcpyFlag=selection;
    printf("Please wait\n");
    glutPostRedisplay();
  }
}


void uncut(int surfFlagBuffer)
{
  char addDispFlagLocal=0;

  if(!inpformat) return;

  if(getSetNr("-qcut")>-1)
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    zap("-qcut");
    if(addDispFlagLocal==2) { addDispToCoordinates(node); }
    surfFlag=surfFlagBuffer;
    glutSetWindow( w1);

    /* the following lines must not appear in ConfigureAndShowWindow_Load(). Else each time called all is displayed instead of a certain set */
    elemEdgeFlag=1;
    if(vectorFlag) pre_view("vector off");
    if(surfFlag)  plot("fv all");
    else          plot("ev all");
  }
}


void selectCutNode( int selection )
{
  static int surfFlagBuffer=1;

  pre_view("elem");
  switch (selection) {
  case 1:
    cutFlag=1; 
    printf("\n Select 1st node with 'left mouse button'\n");
    break;
  case 2:
    cutFlag=2; 
    printf("\n Select 2nd node with 'left mouse button'\n");
    break;
  case 3:
    cutFlag=3; 
    printf("\n Select 3rd node with 'left mouse button'\n");
    surfFlagBuffer=surfFlag;
    break;
  case 4:
    uncut(surfFlagBuffer);
    break;
  case 5:
    cutFlag=4; 
    printf("\n Select one node with 'left mouse button'\n");
    break;
  }
}



void selectGraphMode( int selection )
{
  int i,j,n;
  char buffer[MAX_LINE_LENGTH];

  pre_view("elem");
  switch (selection) {
  case 1:
    printf("\n Values over length. Select nodes with 'left mouse button' then quit with 'right mouse button'\n");
    graphFlag=1; 
    break;
  case 2:
    if(!sequenceFlag)
    {
      printf(" No sequence was defined. Define a sequence (menu: animate->toggle dataset sequence) then start again\n");
      return;
    }
    printf("\n Values over datasets. Select nodes with 'left mouse button' then quit with 'right mouse button'\n");
    graphFlag=2; 
    break;
  case 3:
    if(!sequenceFlag)
    {
      printf(" No sequence was defined. Define a sequence (menu: animate->toggle dataset sequence) then start again\n");
      return;
    }
    printf("\n Values over time. Select nodes with 'left mouse button' then quit with 'right mouse button'\n");
    graphFlag=3; 
    break;
  }
  sprintf(buffer,"se %s", specialset->copy);
  pre_del(buffer);
  sprintf(buffer,"%s", specialset->plot2d);
  i=getSetNr(buffer);
  if(i>-1)
  {
    j=pre_seta(specialset->copy, "i", 0);
    set[j].type=1;
    for(n=0; n<set[i].anz_n; n++) seta(j,"n",set[i].node[n]);
    sprintf(buffer,"se %s", specialset->plot2d);
    pre_del(buffer);
  }
  sprintf(buffer,"%s s", specialset->plot2d);

  /* next time the left mouse-button is used, functon qsequence is called and nodes are stored in specialset->copy */
  /* next time the right mouse-button is used, functon qgraph is called and a graph is created with nodes in specialset->copy */
}



void showHelp( int selection )
{
  char buffer[MAX_LINE_LENGTH];

  switch (selection) {
  case 1:
    help();
    break;
  case 2:
    sprintf( buffer, "%s %s& ", browser, helpfile[0]);
    printf("starting help: %s %s\n", browser, helpfile[0]);
    system (buffer);
    break;
  case 3:
    sprintf( buffer, "%s %s& ", browser, helpfile[1]);
    printf("starting help: %s %s\n", browser, helpfile[1]);
    system (buffer);
    break;
  case 4:
    sprintf( buffer, "%s %s& ", browser, helpfile[2]);
    printf("starting help: %s %s\n", browser, helpfile[2]);
    system (buffer);
    break;
  }
}



void updateDispLists()
{
#if TEST
  printf ("in updateDispLists drawMode=%d \n",drawMode );
#endif 
  if(!inpformat) return;

  glutSetWindow( w1);
  /* update the element and face normals */
  if (drawMode==1)
  {
    if (surfFlag)
    {
      if (lcase[cur_lc].irtype == 3) printf("ERROR: Element-results are not supported so far!\n");
      else drawDispList( list_surf_load, 'f', node, colNr );
    }
    else        
    {
      if (lcase[cur_lc].irtype == 3) printf("ERROR: Element-results are not supported so far!\n");
      else drawDispList( list_elem_load, 'e', node, colNr );
    }
  }
  else if (drawMode==2)
  {
    if (surfFlag)
    {
      getFaceNormalen( face, node, anz );
      drawDispList( list_surf_light, 'f', node, colNr );
    }
    if (!surfFlag)
    {
      getElemNormalen( e_enqire, node, anz->e );
      drawDispList( list_elem_light, 'e', node, colNr );
    }
  }

  if (modelEdgeFlag) drawModelEdges( list_model_edges, basCol[0], 2., anz->g, node );
  if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
  else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );
#if TEST
  printf (" end updateDispLists drawMode=%d \n",drawMode );
#endif 
}



void ConfigureAndShowWindow_Light()
{
  int j;
#if TEST
  printf(" in ConfigureAndShowWindow_Light\n");
#endif 
  if(!inpformat) return;

  if(sequenceFlag)
  {
    ConfigureAndShowWindow_Sequence(1);
    return;
  }

  glutPostRedisplay();
 reselect:;
  glutSetWindow( w0);
  if (animFlag==0) { glutDisplayFunc ( DrawMenuLight ); DrawMenuLight();  }
  else             
  {
    if(drawMode==2)
    {
      /* change the color to "illuminated" */
      for (j=0; j<anzGeo->psets; j++ )
      {
        if((pset[j].type[0]=='e')||(pset[j].type[0]=='f'))
          pset[j].type[1]=0;
      }
      glutDisplayFunc ( DrawMenuAnimate ); DrawMenuAnimate();
    }
    else { glutDisplayFunc ( DrawMenuLoad ); DrawMenuLoad(); }
  }

  glutSetWindow( w1 );
  glShadeModel ( GL_FLAT );
#if INX_MODE
  defineColIndexes_light();
#endif 
  activWindow = w1;
  if (animFlag==0)
  {
    /* reset entities */
    if(surfFlag) sprintf(buffer,"f all %c", entity_k[DEF_COL]);
    else sprintf(buffer,"e all %c", entity_k[DEF_COL]);
    plot(buffer);

    /* change to disp-lists */
    drawMode=2;
    updateDispLists();
    glutDisplayFunc ( DrawGraficLight );
    DrawGraficLight();
  }
  else if (animFlag==1)
  {
    if((drawMode!=2)&&(cur_entity>=lcase[pre_lc].ncomps))
    {
      printf(" Warning: No valid entity or shaded view selected\n\n");
      cur_entity=lcase[pre_lc].ncomps-1;
      drawMode=2;
      goto reselect;
    }
    cur_lc=pre_lc;

    printf ("\nDataset:%d name= %s\n", cur_lc+1, lcase[cur_lc].name);

    glDeleteLists( (GLuint)list_animate_light, (GLint)range_animate_light );
    list_animate_light = glGenLists( (GLint)(anim_steps+1) );
    glDeleteLists( (GLuint)list_model_edges, (GLint)range_animate_light );
    list_model_edges = glGenLists( (GLint)(anim_steps+1) );
    glDeleteLists( (GLuint)list_surf_edges, (GLint)range_animate_light );
    list_surf_edges = glGenLists( (GLint)(anim_steps+1) );
    glDeleteLists( (GLuint)list_elem_edges, (GLint)range_animate_light );
    list_elem_edges = glGenLists( (GLint)(anim_steps+1) );
    range_animate_light = anim_steps+1;

    if((anim_alfa=(int *)realloc((int *)anim_alfa, (range_animate_light+1)*sizeof(char *)))==NULL)
    { printf("\n\n ERROR: malloc failure\n\n" ); exit(1); }
    
    updateDispLists();
    calcAnimation( anim_steps, anim_faktor, anim_alfa, halfperiod, centerNode, anz, node, e_enqire, lcase, cur_lc, scale, surfFlag, colNr, steps );
    glutDisplayFunc ( DrawGraficAnimate );
    DrawGraficAnimate();
  }
  glutSetWindow( w2);
  DrawAxes();

}



void ConfigureAndShowWindow_Plot( void )
{
  int i;
#if TEST
  printf(" in ConfigureAndShowWindow_Plot\n");
#endif 

  if(!inpformat) return;

  for(i=0; i<anzGeo->psets; i++)
  {
   if ((pset[i].type[1]=='v')&&(anz->l))
   {
#if INX_MODE
    defineColIndexes_load();
    nodalDataset( cur_entity, cur_lc, anz, scale, node, lcase, colNr, offset, maxIndex, steps, 1 );
#endif 
#if TEX_MODE
    defineColTextur_load();
    nodalDataset( cur_entity, cur_lc, anz, scale, node, lcase, colNr, 1 );
#endif
    glutSetWindow( w0);
    glutDisplayFunc ( DrawMenuLoad );
    DrawMenuLoad();
    //glutSetWindow( w1 );
    //glShadeModel ( GL_SMOOTH );
    break;
   }
  }
  if (i==anzGeo->psets)
  {
    glutSetWindow( w0);
    glutDisplayFunc ( DrawMenuSet );
    DrawMenuSet();
  }
  glutSetWindow( w1);
  glutDisplayFunc ( DrawPickedItems );
  DrawPickedItems();
  glutSetWindow( w2);
  DrawAxes();
}



void ConfigureAndShowWindow_Load( void )
{
  if(sequenceFlag)
  {
    if(inpformat) ConfigureAndShowWindow_Sequence(0);
    else { printf("ERROR in ConfigureAndShowWindow_Load: not possible in -bg mode.\n"); }
    return;
  }

  if(inpformat)
  {
    glutSetWindow( w1 );
    //glShadeModel ( GL_SMOOTH );

#if INX_MODE
    defineColIndexes_load();
#endif
#if TEX_MODE
    defineColTextur_load();
#endif
  }

  if (lcase[cur_lc].irtype == 3) /* element data */
  {
    elementDataset( cur_entity, cur_lc, anz, scale, lcase, offset, maxIndex, steps );
  }
  else
  {
#if INX_MODE
    nodalDataset( cur_entity, cur_lc, anz, scale, node, lcase, colNr, offset, maxIndex, steps, 1 );
#endif 
#if TEX_MODE
    nodalDataset( cur_entity, cur_lc, anz, scale, node, lcase, colNr, 1 );
#endif
  }

  if(inpformat)
  {
    /* change to disp-lists */
    drawMode=1;
    glutSetWindow( w0);
    glutDisplayFunc ( DrawMenuLoad );
    DrawMenuLoad();

    updateDispLists();

    glutSetWindow( w1);
    glutDisplayFunc ( DrawGraficLoad );
    DrawGraficLoad();
  }
}



void ConfigureAndShowWindow_Sequence( int dispFlag )
{
  int i,j, n;
  int nmax=0, nmin=0;
  double vmax=-MAX_FLOAT, vmin=MAX_FLOAT;

  if(!inpformat) return;

  /* make sure the 1st frame uses the 1st ds */
  animList=0;

  glDeleteLists( (GLuint)list_animate_light, (GLint)range_animate_light );
  list_animate_light = glGenLists( (GLint)(dsSequence.nds) );
  glDeleteLists( (GLuint)list_model_edges, (GLint)range_animate_light );
  list_model_edges = glGenLists( (GLint)(dsSequence.nds) );
  glDeleteLists( (GLuint)list_surf_edges, (GLint)range_animate_light );
  list_surf_edges = glGenLists( (GLint)(dsSequence.nds) );
  glDeleteLists( (GLuint)list_elem_edges, (GLint)range_animate_light );
  list_elem_edges = glGenLists( (GLint)(dsSequence.nds) );
  range_animate_light = dsSequence.nds;

  /* load the loadcases for the sequence, has to be loaded prior to lcase allocation because the ncomps have to be extended before */

  printf("Loading data, please wait\n");

  for(i=0; i<dsSequence.nds; i++)
  {
    printf("ds[%d]:%d %s\n",i+1,dsSequence.ds[i]+1, lcase[dsSequence.ds[i]].name );

    /* check if the data of the specified lcase (Dataset) are already available */
    if (!lcase[dsSequence.ds[i]].loaded)
    {
      if( pre_readfrdblock(copiedNodeSets, dsSequence.ds[i], anz, node, lcase )==-1) 
      {
        printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", dsSequence.ds[i]+1); 
        return;
      }
      calcDatasets( dsSequence.ds[i], anz, node, lcase );
      recompileEntitiesInMenu(dsSequence.ds[i]);
    }

    if(vmax<lcase[dsSequence.ds[i]].max[cur_entity])
    { vmax=lcase[dsSequence.ds[i]].max[cur_entity]; nmax=lcase[dsSequence.ds[i]].nmax[cur_entity]; }
    if(vmin>lcase[dsSequence.ds[i]].min[cur_entity])
    { vmin=lcase[dsSequence.ds[i]].min[cur_entity]; nmin=lcase[dsSequence.ds[i]].nmin[cur_entity]; }
  }
  /* set to the max-vals in range */
  if((scale->smin ==scale->smax )&&(scale->lock!='l'))
  {
    scale->smin = vmin;
    scale->smax = vmax;
  }
  
  if(drawMode==2) /* light */
    //if((compare(lcase[dsSequence.ds[0]].name, "DISP", 4)==4)&&(drawMode==2)&&(lcase[dsSequence.ds[0]].ictype[0]!= 12)) /* light */
  {
    /* switch the psets to the shaded mode */
    for (j=0; j<anzGeo->psets; j++ )
    {
      if((pset[j].type[0]=='e')||(pset[j].type[0]=='f'))
        pset[j].type[1]=0;
    }
    
    lightFlag=1;
    glutSetWindow( w1 );
#if INX_MODE
    defineColIndexes_light();
#endif 

    if(lcase_animList<0)
    {
      /* create an additional lcase after for the vector length of all steps */
      if ( (lcase = (Datasets *)realloc((Datasets *)lcase, (anz->l+2) * sizeof(Datasets))) == NULL )
      { printf("\n\n ERROR: malloc failure\n\n" ); exit(1); }

      lcase[anz->l].ncomps = dsSequence.nds;
      lcase[anz->l].irtype = 1;
      lcase[anz->l].npheader = 0;
      lcase[anz->l].fileptr = NULL;
      lcase[anz->l].loaded = 1;
      lcase[anz->l].value=0;
      strcpy(lcase[anz->l].analysis_name,"");
      strcpy(lcase[anz->l].dataset_name,"");
      strcpy(lcase[anz->l].dataset_text,"");
      lcase[anz->l].step_number=0;
      lcase[anz->l].analysis_type=1;
  
      if ( (lcase[anz->l].nmax = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].nmin = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].max = (float *)malloc( lcase[anz->l].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].min = (float *)malloc( lcase[anz->l].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].dat = (float **)malloc( lcase[anz->l].ncomps * sizeof(float *))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].compName = (char **)malloc( lcase[anz->l].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].icname = (char **)malloc( lcase[anz->l].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      for(j=0; j<lcase[anz->l].ncomps; j++)
      {
        if ( (lcase[anz->l].dat[j] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: malloc failure\n\n" );	               
        if ( (lcase[anz->l].compName[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        if ( (lcase[anz->l].icname[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        lcase[anz->l].max[j]=-MAX_FLOAT;
        lcase[anz->l].min[j]=MAX_FLOAT;
      }
      if ( (lcase[anz->l].menu = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].ictype = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].icind1 = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].icind2 = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].iexist = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
  
      for(j=0; j<lcase[anz->l].ncomps; j++)
      {
        lcase[anz->l].menu[j] = 1;
        lcase[anz->l].ictype[j] = 2;
        lcase[anz->l].icind1[j] = j+1;
        lcase[anz->l].icind2[j] = 0;
        lcase[anz->l].iexist[j] = 0;
        sprintf(lcase[anz->l].compName[j], "step");
      }
      lcase_animList=anz->l;
    }
    else
    {
      /* realloc the additional lcase after for the vector length of all steps */
      for(j=0; j<lcase[lcase_animList].ncomps; j++)
      {
        free(lcase[lcase_animList].dat[j]);
        free(lcase[lcase_animList].compName[j]);
        free(lcase[lcase_animList].icname[j]);
      }
  
      lcase[lcase_animList].ncomps = dsSequence.nds;
      lcase[lcase_animList].irtype = 1;
      lcase[lcase_animList].npheader = 0;
      lcase[lcase_animList].fileptr = NULL;
      lcase[lcase_animList].loaded = 1;

      if ( (lcase[lcase_animList].nmax = (int *)realloc( lcase[lcase_animList].nmax, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].nmin = (int *)realloc( lcase[lcase_animList].nmin, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].max = (float *)realloc( lcase[lcase_animList].max, lcase[lcase_animList].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].min = (float *)realloc( lcase[lcase_animList].min, lcase[lcase_animList].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].dat = (float **)realloc( lcase[lcase_animList].dat, lcase[lcase_animList].ncomps * sizeof(float *))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].compName = (char **)realloc( lcase[lcase_animList].compName, lcase[lcase_animList].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].icname = (char **)realloc( lcase[lcase_animList].icname, lcase[lcase_animList].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      for(j=0; j<lcase[lcase_animList].ncomps; j++)
      {
        if ( (lcase[lcase_animList].dat[j] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: malloc failure\n\n" );	               
        if ( (lcase[lcase_animList].compName[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        if ( (lcase[lcase_animList].icname[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        lcase[lcase_animList].max[j]=-MAX_FLOAT;
        lcase[lcase_animList].min[j]=MAX_FLOAT;
      }
      if ( (lcase[lcase_animList].menu = (int *)realloc( lcase[lcase_animList].menu, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].ictype = (int *)realloc( lcase[lcase_animList].ictype, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].icind1 = (int *)realloc( lcase[lcase_animList].icind1, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].icind2 = (int *)realloc( lcase[lcase_animList].icind2, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].iexist = (int *)realloc( lcase[lcase_animList].iexist, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
  
      for(j=0; j<lcase[lcase_animList].ncomps; j++)
      {
        lcase[lcase_animList].menu[j] = 1;
        lcase[lcase_animList].ictype[j] = 2;
        lcase[lcase_animList].icind1[j] = j+1;
        lcase[lcase_animList].icind2[j] = 0;
        lcase[lcase_animList].iexist[j] = 0;
        sprintf(lcase[lcase_animList].compName[j], "step");
      }
    }
    calcSequence( dsSequence, anim_faktor, halfperiod, centerNode, anz, node, e_enqire, lcase, scale, surfFlag, colNr, steps, lcase_animList, dispFlag);
    glutSetWindow( w0);

    strcpy(lcase[lcase_animList].name,lcase[dsSequence.ds[0]].name);
    strcpy(lcase[lcase_animList].dataset_text,lcase[dsSequence.ds[0]].dataset_text);
    lcase[lcase_animList].step_number=lcase[anz->l-1].step_number+1;
    glutDisplayFunc ( DrawMenuAnimate );
    DrawMenuAnimate();
    activWindow = w1;
  }
  else /* load */
  {
    drawMode=1;
    lightFlag=0;
    glutSetWindow( w1 );
    //glShadeModel ( GL_SMOOTH );
#if INX_MODE
    defineColIndexes_load();
#endif 
#if TEX_MODE
    defineColTextur_load();
#endif

    if(lcase_animList<0)
    {
      /* create an additional lcase after for the vector length of all steps */
      if ( (lcase = (Datasets *)realloc((Datasets *)lcase, (anz->l+2) * sizeof(Datasets))) == NULL )
      { printf("\n\n ERROR: malloc failure\n\n" ); exit(1); }

      lcase[anz->l].ncomps = dsSequence.nds;
      lcase[anz->l].irtype = 1;
      lcase[anz->l].npheader = 0;
    
      if ( (lcase[anz->l].nmax = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].nmin = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].max = (float *)malloc( lcase[anz->l].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].min = (float *)malloc( lcase[anz->l].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].dat = (float **)malloc( lcase[anz->l].ncomps * sizeof(float *))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].compName = (char **)malloc( lcase[anz->l].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].icname = (char **)malloc( lcase[anz->l].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      for(j=0; j<lcase[anz->l].ncomps; j++)
      {
        if ( (lcase[anz->l].dat[j] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: malloc failure\n\n" );	               
        if ( (lcase[anz->l].compName[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        if ( (lcase[anz->l].icname[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        lcase[anz->l].max[j]=-MAX_FLOAT;
        lcase[anz->l].min[j]=MAX_FLOAT;
      }
      if ( (lcase[anz->l].menu = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].ictype = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].icind1 = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].icind2 = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (lcase[anz->l].iexist = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
    
      for(j=0; j<lcase[anz->l].ncomps; j++)
      {
        lcase[anz->l].menu[j] = 1;
        lcase[anz->l].ictype[j] = 2;
        lcase[anz->l].icind1[j] = j+1;
        lcase[anz->l].icind2[j] = 0;
        lcase[anz->l].iexist[j] = 0;
      }
      lcase_animList=anz->l;
    }
    else
    {
      /* realloc the additional lcase after for the vector length of all steps */
      for(j=0; j<lcase[lcase_animList].ncomps; j++)
      {
        free(lcase[lcase_animList].dat[j]);
        free(lcase[lcase_animList].compName[j]);
        free(lcase[lcase_animList].icname[j]);
      }
  
      lcase[lcase_animList].ncomps = dsSequence.nds;
      lcase[lcase_animList].irtype = 1;
      lcase[lcase_animList].npheader = 0;
    
      if ( (lcase[lcase_animList].nmax = (int *)realloc( lcase[lcase_animList].nmax, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].nmin = (int *)realloc( lcase[lcase_animList].nmin, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].max = (float *)realloc( lcase[lcase_animList].max, lcase[lcase_animList].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].min = (float *)realloc( lcase[lcase_animList].min, lcase[lcase_animList].ncomps * sizeof(float))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].dat = (float **)realloc( lcase[lcase_animList].dat, lcase[lcase_animList].ncomps * sizeof(float *))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].compName = (char **)realloc( lcase[lcase_animList].compName, lcase[lcase_animList].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].icname = (char **)realloc( lcase[lcase_animList].icname, lcase[lcase_animList].ncomps * sizeof(char *))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      for(j=0; j<lcase[lcase_animList].ncomps; j++)
      {
        if ( (lcase[lcase_animList].dat[j] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: malloc failure\n\n" );	               
        if ( (lcase[lcase_animList].compName[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        if ( (lcase[lcase_animList].icname[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
           printf("\n\n ERROR: malloc failed\n\n" );
        lcase[lcase_animList].max[j]=-MAX_FLOAT;
        lcase[lcase_animList].min[j]=MAX_FLOAT;
      }
      if ( (lcase[lcase_animList].menu = (int *)realloc( lcase[lcase_animList].menu, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].ictype = (int *)realloc( lcase[lcase_animList].ictype, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].icind1 = (int *)realloc( lcase[lcase_animList].icind1, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].icind2 = (int *)realloc( lcase[lcase_animList].icind2, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
      if ( (lcase[lcase_animList].iexist = (int *)realloc( lcase[lcase_animList].iexist, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failure\n\n" );
    
      for(j=0; j<lcase[lcase_animList].ncomps; j++)
      {
        lcase[lcase_animList].menu[j] = 1;
        lcase[lcase_animList].ictype[j] = 2;
        lcase[lcase_animList].icind1[j] = j+1;
        lcase[lcase_animList].icind2[j] = 0;
        lcase[lcase_animList].iexist[j] = 0;
      }
    }

    calcSequence( dsSequence, anim_faktor, halfperiod, centerNode, anz, node, e_enqire, lcase, scale, surfFlag, colNr, steps, lcase_animList, dispFlag);
    glutSetWindow( w0);

    strcpy(lcase[lcase_animList].name,lcase[dsSequence.ds[0]].name);
    strcpy(lcase[lcase_animList].dataset_text,lcase[dsSequence.ds[0]].dataset_text);
    lcase[lcase_animList].step_number=lcase[anz->l-1].step_number+1;
    glutDisplayFunc ( DrawMenuSequence );
    DrawMenuSequence();
    activWindow = w1;
  }
  lcase[lcase_animList].value=0;
  lcase[lcase_animList].max[0]=vmax;
  lcase[lcase_animList].min[0]=vmin;
  lcase[lcase_animList].nmax[0]=nmax;
  lcase[lcase_animList].nmin[0]=nmin;
  for(n=0; n<lcase[lcase_animList].ncomps; n++)
  {
    lcase[lcase_animList].dat[n][0]=lcase[dsSequence.ds[n]].value;
  }
  //cur_lc=lcase_animList;  // not active because some functions need the original dataset and use cur_lc to find it (cut)
  cur_lc=dsSequence.ds[0];

    glutSetWindow( w1 );
    glShadeModel ( GL_FLAT );
    glutDisplayFunc ( DrawGraficSequence );
    DrawGraficSequence();
}



void ConfigureAndShowWindow_Vector( void )
{
  int i,j,k,n;
  int nmax=0, nmin=0;
  double vmax=-MAX_FLOAT, vmin=MAX_FLOAT;
  
  if(!inpformat) return;
  if(!dsSequence.nds) {  printf("ERROR: No ds selected:%d\n",dsSequence.nds); return; }

  /* make sure the 1st frame uses the 1st ds */
  animList=0;

  drawModelEdges( list_model_edges, basCol[0], 2., anz->g, node );
  if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
  else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );

  glDeleteLists( (GLuint)list_animate_light, (GLint)range_animate_light );
  list_animate_light = glGenLists( (GLint)(dsSequence.nds) );
  range_animate_light = dsSequence.nds;

  lightFlag=0;
#if INX_MODE
  defineColIndexes_load();
#endif 
#if TEX_MODE
  defineColTextur_load();
#endif

  /* load the loadcases for the vectors, has to be loaded prior to lcase allocation because the ncomps have to be extended before */

  printf("Loading data, please wait\n");
  for(i=0; i<dsSequence.nds; i++)
  {
    printf("ds[%d]:%d %s\n",i+1,dsSequence.ds[i]+1, lcase[dsSequence.ds[i]].name );

    /* check if the data of the specified lcase (Dataset) are already available */
    if (!lcase[dsSequence.ds[i]].loaded)
    {
      if( pre_readfrdblock(copiedNodeSets , dsSequence.ds[i], anz, node, lcase )==-1) 
      {
        printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", dsSequence.ds[i]+1); 
        return;
      }
      calcDatasets( dsSequence.ds[i], anz, node, lcase );
      recompileEntitiesInMenu(dsSequence.ds[i]);
    }
  }

  if(lcase_animList<0)
  {
    /* create an additional lcase for the vector length of all steps */
    if ( (lcase = (Datasets *)realloc((Datasets *)lcase, (anz->l+2) * sizeof(Datasets))) == NULL )
      printf("\n\n ERROR: realloc failed, lcase\n\n") ;
    strcpy(lcase[anz->l].name,lcase[dsSequence.ds[0]].name);
    lcase[anz->l].ncomps = dsSequence.nds;
    lcase[anz->l].irtype = 1;
    lcase[anz->l].npheader = 0;
    lcase[anz->l].fileptr = NULL;
    lcase[anz->l].loaded = 1;
    lcase[anz->l].step_number=0;

    if ( (lcase[anz->l].nmax = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].nmin = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].max = (float *)malloc( lcase[anz->l].ncomps * sizeof(float))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].min = (float *)malloc( lcase[anz->l].ncomps * sizeof(float))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].dat = (float **)malloc( lcase[anz->l].ncomps * sizeof(float *))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].compName = (char **)malloc( lcase[anz->l].ncomps * sizeof(char *))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].icname = (char **)malloc( lcase[anz->l].ncomps * sizeof(char *))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    for(j=0; j<lcase[anz->l].ncomps; j++)
    {
      if ( (lcase[anz->l].dat[j] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );	               
      if ( (lcase[anz->l].compName[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
         printf("\n\n ERROR: malloc failed\n\n" );
      if ( (lcase[anz->l].icname[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
         printf("\n\n ERROR: malloc failed\n\n" );
      lcase[anz->l].max[j]=-MAX_FLOAT;
      lcase[anz->l].min[j]=MAX_FLOAT;
    }
    if ( (lcase[anz->l].menu = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].ictype = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].icind1 = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].icind2 = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );
    if ( (lcase[anz->l].iexist = (int *)malloc( lcase[anz->l].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );

    for(j=0; j<lcase[anz->l].ncomps; j++)
    {
      lcase[anz->l].menu[j] = 1;
      lcase[anz->l].ictype[j] = 2;
      lcase[anz->l].icind1[j] = j+1;
      lcase[anz->l].icind2[j] = 0;
      lcase[anz->l].iexist[j] = 0;
    }
    lcase_animList=anz->l;
  }
  else
  {
    /* realloc the additional lcase for the vector length of all steps */
    for(j=0; j<lcase[lcase_animList].ncomps; j++)
    {
      free(lcase[lcase_animList].dat[j]);
      free(lcase[lcase_animList].compName[j]);
      free(lcase[lcase_animList].icname[j]);
    }

    lcase[lcase_animList].ncomps = dsSequence.nds;
    lcase[lcase_animList].irtype = 1;
    lcase[lcase_animList].npheader = 0;
    lcase[lcase_animList].fileptr = NULL;
    lcase[lcase_animList].loaded = 1;

    if ( (lcase[lcase_animList].nmax = (int *)realloc( lcase[lcase_animList].nmax, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].nmin = (int *)realloc( lcase[lcase_animList].nmin, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].max = (float *)realloc( lcase[lcase_animList].max, lcase[lcase_animList].ncomps * sizeof(float))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].min = (float *)realloc( lcase[lcase_animList].min, lcase[lcase_animList].ncomps * sizeof(float))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].dat = (float **)realloc( lcase[lcase_animList].dat, lcase[lcase_animList].ncomps * sizeof(float *))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].compName = (char **)realloc( lcase[lcase_animList].compName, lcase[lcase_animList].ncomps * sizeof(char *))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].icname = (char **)realloc( lcase[lcase_animList].icname, lcase[lcase_animList].ncomps * sizeof(char *))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    for(j=0; j<lcase[lcase_animList].ncomps; j++)
    {
      if ( (lcase[lcase_animList].dat[j] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );	               
      if ( (lcase[lcase_animList].compName[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
         printf("\n\n ERROR: malloc failed\n\n" );
      if ( (lcase[lcase_animList].icname[j] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
         printf("\n\n ERROR: malloc failed\n\n" );
      lcase[lcase_animList].max[j]=-MAX_FLOAT;
      lcase[lcase_animList].min[j]=MAX_FLOAT;
    }
    if ( (lcase[lcase_animList].menu = (int *)realloc( lcase[lcase_animList].menu, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].ictype = (int *)realloc( lcase[lcase_animList].ictype, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].icind1 = (int *)realloc( lcase[lcase_animList].icind1, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].icind2 = (int *)realloc( lcase[lcase_animList].icind2, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    if ( (lcase[lcase_animList].iexist = (int *)realloc( lcase[lcase_animList].iexist, lcase[lcase_animList].ncomps * sizeof(int))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );

    for(j=0; j<lcase[lcase_animList].ncomps; j++)
    {
      lcase[lcase_animList].menu[j] = 1;
      lcase[lcase_animList].ictype[j] = 2;
      lcase[lcase_animList].icind1[j] = j+1;
      lcase[lcase_animList].icind2[j] = 0;
      lcase[lcase_animList].iexist[j] = 0;
    }
  }
  //cur_lc=lcase_animList;  // not active because some functions need the original dataset and use cur_lc to find it (cut)
  cur_lc=dsSequence.ds[0];

  /* store and calculate the vector length */
  for(n=0; n<dsSequence.nds; n++)
  {
    i=dsSequence.ds[n];
    if(v_dim==4) sprintf(lcase[lcase_animList].compName[n], lcase[i].compName[cur_entity]);
    else 
    {
      if(v_dim==3) sprintf(lcase[lcase_animList].compName[n], "v(%s,%s,%s)", lcase[i].compName[entity_v[0]], lcase[i].compName[entity_v[1]], lcase[i].compName[entity_v[2]]);
      else         sprintf(lcase[lcase_animList].compName[n], "v(%s,%s)", lcase[i].compName[entity_v[0]], lcase[i].compName[entity_v[1]]);
      /* delete blanks */
      k=0; for(j=0; j<strlen(lcase[lcase_animList].compName[n]); j++) if(lcase[lcase_animList].compName[n][j]!=' ') { lcase[lcase_animList].compName[n][k++]=lcase[lcase_animList].compName[n][j]; }
      lcase[lcase_animList].compName[n][k]='\0';
    }
    /* select the vector length (color plot) */
    if (lcase[i].irtype == 3) { printf("Element results are not supported\n"); return; }

    for(j=0; j<anz->n; j++)
    {
      if(node[node[j].nr].pflag==-1) continue;
      if(v_dim==4)
      {
        lcase[lcase_animList].dat[n][node[j].nr]=lcase[i].dat[entity_v[3]][node[j].nr];
      }
      else
      {
        lcase[lcase_animList].dat[n][node[j].nr]=0.;
        for(k=0; k<v_dim; k++)
          lcase[lcase_animList].dat[n][node[j].nr]+=lcase[i].dat[entity_v[k]][node[j].nr]*lcase[i].dat[entity_v[k]][node[j].nr];
        lcase[lcase_animList].dat[n][node[j].nr]=sqrt(lcase[lcase_animList].dat[n][node[j].nr]);
      }
    }
  }

  /* max and min for plotting */
  for(k=0; k<lcase[lcase_animList].ncomps; k++)
  {
    for(j=0; j<anz->n; j++)
    {
      if(node[node[j].nr].pflag==-1) continue;
      if(lcase[lcase_animList].dat[k][node[j].nr] > vmax)
      {
        vmax=lcase[lcase_animList].dat[k][node[j].nr];
        nmax=node[j].nr;
      }
      if(lcase[lcase_animList].dat[k][node[j].nr] < vmin)
      {
        vmin=lcase[lcase_animList].dat[k][node[j].nr];
        nmin=node[j].nr;
      }
    }
  }
  //printf (" maxval:%e at node:%d\n minval:%e at node:%d  cur_lc:%d cur_entity:%d\n", vmax, nmax, vmin, nmin, cur_lc, cur_entity);

  /* set to the max-vals in range */
  if(scale->smin ==scale->smax )
  {
    scale->smin = vmin;
    scale->smax = vmax;
  }

  /* save the overall values for the drawing */ 
  lcase[lcase_animList].max[0] =vmax;
  lcase[lcase_animList].nmax[0]=nmax;
  lcase[lcase_animList].min[0] =vmin;
  lcase[lcase_animList].nmin[0]=nmin;
    
  if(vmax*vmax>vmin*vmin) v_factor=1./vmax;
  else v_factor=1./vmin;

  /* prepare the color values */
  for(k=0; k<lcase[lcase_animList].ncomps; k++)
  {
#if INX_MODE
    nodalDataset( k, lcase_animList, anz, scale, node, lcase, colNr, offset, maxIndex, steps, 0 );
#endif 
#if TEX_MODE
    nodalDataset( k, lcase_animList, anz, scale, node, lcase, colNr, 0 );
#endif 

    lcase[lcase_animList].dat[k][0]=lcase[dsSequence.ds[k]].value;

    glutSetWindow( w1);
    if ( (list_animate = (GLuint *)realloc( list_animate, (k+1) * sizeof(GLuint))) == NULL )
      printf("\n\n ERROR: realloc failure\n\n" );
    list_animate[k]=list_animate_light+k;
    if (surfFlag)   drawDispList( list_animate[k], 'f', node, colNr );
    if (!surfFlag)  drawDispList( list_animate[k], 'e', node, colNr );
  }
  lcase[lcase_animList].value=0;
  lcase[lcase_animList].dataset_text[0]=0;
  strcpy(lcase[lcase_animList].name,lcase[dsSequence.ds[0]].name);
  lcase[lcase_animList].step_number=lcase[anz->l-1].step_number+1;

  glutSetWindow( w0);
  glutDisplayFunc ( DrawMenuSequence );
  DrawMenuSequence();
  glutSetWindow( w1 );
  //glShadeModel ( GL_SMOOTH );
  defineColTextur_load();
  glutDisplayFunc ( DrawGraficSequence );
  DrawGraficSequence();
}



int minus( char *record )
{
  int   setNr;
  int  length, i, flag=0;
  char typ[MAX_LINE_LENGTH];
  char setnam[MAX_LINE_LENGTH];

  if(!inpformat) return(-1);

  length = sword( record, typ );
  length = 1+length + sword( &record[length], setnam );
  if   (typ[0]=='m') typ[0]='e';

  operateAlias( setnam, "se" );
  setNr=getSetNr(setnam);
  if (setNr<0)
  {
    printf (" minus: set:%s does not exist\n", setnam);
    return (-1);
  }

  /* look for the pset to remove */
  for(i=0; i<anzGeo->psets; i++)
  {
    if(pset[i].nr==setNr)
    {
      if(pset[i].type[0]==typ[0])
      {
        printf (" set:%s type:%s removed\n", setnam, pset[i].type);
        strcpy( pset[i].type, " ");
      }
    }
    updateDispLists(); 
    flag=1;
  }
  if(flag==0) printf (" set:%s is not on the screen\n", setnam);

  ConfigureAndShowWindow_Plot();
  return(1);
}



void zoom(double xcur0, double ycur0, double xcur1, double ycur1)
{
  double  dx_cur, dy_cur, dso;

  if(!inpformat) return;

      dx_cur= xcur1-xcur0;
      dx_cur= sqrt( dx_cur*dx_cur );
      dy_cur= ycur1-ycur0;
      dy_cur= sqrt( dy_cur*dy_cur );
      dso=ds;
      if (dx_cur<dy_cur) ds*=dy_cur/2.;
      else ds*=dx_cur/2.;
      dtx=(dtx- (xcur1+xcur0)/2.)*dso/ds ;
      dty=(dty- (ycur1+ycur0)/2.)*dso/ds ;
      zoomFlag=0;
      glutPostRedisplay();
      updateDispLists();
}


void pre_rot( char *record)
{
  int  length;
  double angle;
  char type[MAX_LINE_LENGTH];

  if(!inpformat) return;

  length = sword( record, type );
  angle = atof( &record[length+1]);
  if (type[0]=='u') rot_u(angle);
  else if (type[0]=='r') rot_r(angle);
  else if (type[0]=='c') rot_c(angle);
  else if (type[0]=='d') rot_u(-angle);
  else if (type[0]=='l') rot_r(-angle);
  else if (type[0]=='x') rot_x(1.);
  else if (type[0]=='y') rot_y(1.);
  else if (type[0]=='z') rot_z(1.);
  else if (type[0]=='-')
  {
    if (type[1]=='x') rot_x(-1.);
    else if (type[1]=='y') rot_y(-1.);
    else if (type[1]=='z') rot_z(-1.);
    else errMsg(" rot type:%s not known\n", type);
  }
  else errMsg(" rot type:%s not known\n", type);
  glutSetWindow( w2 );
  glutPostRedisplay();
  glutSetWindow( w1);
  glutPostRedisplay();
}



int plot( char *record )
{
  int  i,n,setNr;
  int  length,index;
  char typ[MAX_LINE_LENGTH];
  char setnam[MAX_LINE_LENGTH];
  char col[MAX_LINE_LENGTH];

  if(!inpformat) return(-1);

  col[0]=0;
  length = sword( record, typ );
  length = 1+length + sword( &record[length], setnam );
  length = 1+length + sword( &record[length], col );
  if (typ[0]=='m') typ[0]='e';

  operateAlias( setnam, "se" );

  /* get the bad elements in set */
  if (typ[1]=='q')
  {
    i=calcBadElements(setnam);
    if (i>0) printf("found %d bad elements in set:%s (stored in set:%s)\n", i,setnam, specialset->njby);
    else printf("found no bad element in set:%s\n", setnam);
    strcpy(setnam,specialset->njby);
    typ[1]=typ[2];
  }
  setNr=getSetNr(setnam);
  if (setNr<0)
  {
    printf (" plot: set:%s does not exist\n", setnam);
    return (-1);
  }

  if ((pset = (Psets *)realloc( (Psets *)pset, (1)*sizeof(Psets)) ) == NULL )
  {
    printf(" ERROR: realloc failure in plot, pset not installed\n\n");
    return(-1);
  }
  anzGeo->psets=1;

  n=0;
  for(i=0; i<SET_COLS; i++) if(col[0]==entity_k[i]) { pset[0].col=i; n=1; }
  if (n==0)
  { 
  if        (typ[0]=='n') pset[0].col=3;
  else if   (typ[0]=='e') pset[0].col=4;
  else if   (typ[0]=='f') pset[0].col=6;
  else if   (typ[0]=='p') pset[0].col=5;
  else if   (typ[0]=='l') pset[0].col=foregrndcol;
  else if   (typ[0]=='s') pset[0].col=7;
  else if   (typ[0]=='b') pset[0].col=4;
  else if   (typ[0]=='L') pset[0].col=foregrndcol;
  else if   (typ[0]=='S') pset[0].col=2;
  else                    pset[0].col=6;
  }
  pset[0].nr= setNr;
  strcpy( pset[0].type, typ);

  drawMode=4;
  if((typ[0]=='e')||(typ[0]=='f'))
  {
    glutSetWindow( w1);
    glShadeModel ( GL_FLAT );
    if(typ[0]=='e') surfFlag=0;
#if INX_MODE
    defineColIndexes_light();
#endif 
    updateDispLists();
  }

  /* show links to related sets */
  if((typ[0]=='n')||(typ[0]=='f'))
  {
    if(set[setNr].anz_se)
    {
      index=2;
      for(i=1; i<anz->sets; i++) if(!set[i].type) set[i].index=index++;
      printf("->[ ");
      for(i=0; i<set[setNr].anz_se; i++) 
        printf("%s(%d) ",set[set[setNr].set[i]].name,set[set[setNr].set[i]].index);
      printf("]\n");
    }
  }

  ConfigureAndShowWindow_Plot();
  return(1);
}



int plus( char *record )
{
  int  i,n, setNr;
  int  length,index;
  char typ[MAX_LINE_LENGTH];
  char setnam[MAX_LINE_LENGTH];
  char col[MAX_LINE_LENGTH];

  if(!inpformat) return(-1);

  col[0]=0;
  length = sword( record, typ );
  length = 1+length + sword( &record[length], setnam );
  length = 1+length + sword( &record[length], col );
  if (typ[0]=='m') typ[0]='e';

  operateAlias( setnam, "se" );

  /* get the bad elements in set */
  if (typ[1]=='q')
  {
    i=calcBadElements(setnam);
    if (i>0) printf("found %d bad elements in set:%s (stored in set:%s)\n", i,setnam, specialset->njby);
    strcpy(setnam,specialset->njby);
    typ[1]=typ[2];
  }
  setNr=getSetNr(setnam);
  if (setNr<0)
  {
    printf (" plus: set:%s does not exist\n", setnam);
    return (-1);
  }
  
  if ((pset = (Psets *)realloc( (Psets *)pset, (anzGeo->psets+1)*sizeof(Psets)) ) == NULL )
  {
    printf(" ERROR: realloc failure in plot, pset not installed\n\n");
    return(-1);
  }

  n=0;
  for(i=0; i<SET_COLS; i++) if(col[0]==entity_k[i]) { pset[anzGeo->psets].col=i; n=1; }
  if (n==0)
  { 
  if        (typ[0]=='n') pset[anzGeo->psets].col=3;
  else if   (typ[0]=='e') pset[anzGeo->psets].col=4;
  else if   (typ[0]=='f') pset[anzGeo->psets].col=6;
  else if   (typ[0]=='p') pset[anzGeo->psets].col=5;
  else if   (typ[0]=='l') pset[anzGeo->psets].col=foregrndcol;
  else if   (typ[0]=='s') pset[anzGeo->psets].col=7;
  else if   (typ[0]=='b') pset[anzGeo->psets].col=4;
  else if   (typ[0]=='L') pset[anzGeo->psets].col=foregrndcol;
  else if   (typ[0]=='S') pset[anzGeo->psets].col=2;
  else                    pset[anzGeo->psets].col=6;
  }
  pset[anzGeo->psets].nr= setNr;
  strcpy( pset[anzGeo->psets].type, typ);
  anzGeo->psets++;
  drawMode=4;
  if((typ[0]=='e')||(typ[0]=='f'))
  {
    if(typ[0]=='e') surfFlag=0;
#if INX_MODE
    defineColIndexes_light();
#endif 
    updateDispLists();
  }

  /* show links to related sets */
  if((typ[0]=='n')||(typ[0]=='f'))
  {
    if(set[setNr].anz_se)
    {
      index=2;
      for(i=1; i<anz->sets; i++) if(!set[i].type) set[i].index=index++;
      printf("->[ ");
      for(i=0; i<set[setNr].anz_se; i++) 
        printf("%s(%d) ",set[set[setNr].set[i]].name,set[set[setNr].set[i]].index);
      printf("]\n");
    }
  }

  ConfigureAndShowWindow_Plot();
  return(1);
}



void changeAnimation( int selection )
{
  /* more options in separate submenus, not listed here */
  if(anz->l<1) return;

  switch (selection) {
  case 1:
      animFlag=1;
      ConfigureAndShowWindow_Light();
    break;
  case 2:
      animFlag=1;
      halfperiod=!halfperiod;
      ConfigureAndShowWindow_Light();
    break;
  case 3:
      modelEdgeFlag_Static=!modelEdgeFlag_Static;
      if (modelEdgeFlag_Static) /* copy from updateDispLists() */
        drawModelEdges( list_model_edges, basCol[0], 2., anz->g, node );
    break;
  case 4:
      elemEdgeFlag_Static=!elemEdgeFlag_Static;
      if (elemEdgeFlag_Static)
      {
        if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
        else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );
      }
    break;
  case 5:
      seq_nlc=0;
      seqLC[1]=0;
      seqLC[2]=anz->l-1;
      animList=0;
      sequenceFlag=!sequenceFlag;
      if(sequenceFlag)
      {
        printf(" choose 1st Dataset (optionally 2nd and last Dataset) and the entity\n");
      }
      else
      {
        ConfigureAndShowWindow_Load();
        printf(" sequence off\n");
      }
    break;
  }
}



void newTimePerPeriod( int selection )
{
  switch (selection) {
  case 1:
      time_per_period=1;
    break;
  case 2:
      time_per_period=1000;
    break;
  case 3:
      time_per_period=1200;
    break;
  case 4:
      time_per_period=1500;
    break;
  case 5:
      time_per_period=2000;
    break;
  case 6:
      time_per_period=5000;
    break;
  }
  redraw();
}



void tuneAnimation( int selection )
{
  switch (selection) {
  case 0:
    anim_faktor=1.;
    break;
  case 1:
    anim_faktor*= 10.;
    break;
  case 2:
    anim_faktor*= 5.;
    break;
  case 3:
    anim_faktor*= 2;
    break;
  case 4:
    anim_faktor/= 2;
    break;
  case 5:
    anim_faktor/= 5.;
    break;
  case 6:
    anim_faktor/= 10.;
    break;
  }
  /* remove the displacements and use the scaled ones */
  if(addDispFlag==1)
  {
     addDispToCoordinates(node);
     addDispToCoordinates(node);
  }
  redraw();
}



void stepsAnimation( int selection )
{
  anim_steps=selection;
  redraw();
}



void redraw(void)
{
  if(!inpformat) return;

  //printf("drawMode:%d animFlag:%d\n", drawMode, animFlag);
  if ((drawMode==1)&&(!animFlag))
  {
    ConfigureAndShowWindow_Load();
  }
  if ((drawMode==1)&&(animFlag))
  {
    ConfigureAndShowWindow_Load();
    ConfigureAndShowWindow_Light();
  }
  if (drawMode==2)
  {
    ConfigureAndShowWindow_Light();
  }
  if (drawMode==4)
  {
    ConfigureAndShowWindow_Plot();
    if (animFlag) ConfigureAndShowWindow_Light();
  }
  if (drawMode==5)
  {
    ConfigureAndShowWindow_Vector();
  }
  glutPostRedisplay();
}



void createDsSequence(int seq_nlc, int *seqLC)
{
  int i,j, freq=0;

  /* remove the displacements  */
  if(addDispFlag==1) addDispToCoordinates(node);

  dsSequence.nds=0;
  if( seq_nlc<3) seqLC[2]=anz->l-1;
  if( seq_nlc<2) return;
 
  //printf("seqLC %d %d %d\n",seqLC[0],seqLC[1],seqLC[2]);

  /* determine the frequency */
  for(i=seqLC[0]; i<=seqLC[2]; i++)
  {
    if(i>=seqLC[1]) break;
    if(compareStrings(lcase[i].name, lcase[seqLC[0]].name)>0) freq++;
  }
  //printf("freq:%d\n",freq);

  j=freq-1;
  for(i=seqLC[0]; i<=seqLC[2]; i++)
  {
    if(compareStrings(lcase[i].name, lcase[seqLC[0]].name)>0)
    {
      j++;
      if(j==freq)
      {
        j=0;
        if((dsSequence.ds=(int *)realloc((int *)dsSequence.ds, (dsSequence.nds+1)*sizeof(int)))==NULL) printf("\nERROR realloc in createDsSequence\n\n");
        dsSequence.ds[dsSequence.nds]=i;
        //printf("nr:%d ds:%d i:%d\n", dsSequence.nds, dsSequence.ds[dsSequence.nds], i);
        dsSequence.nds++; 
      }
    }
  }
}



void selectParameter( int selection )
{
  char  buffer[MAX_LINE_LENGTH],  parameter[MAX_LINE_LENGTH];
  
  sscanf(&lcase[pre_lc].pheader[selection-1][6],"%s",parameter ); 
  //printf("%s selected:%d %s\n", lcase[pre_lc].name, selection, parameter );
  sprintf(buffer, "-p nr %s %s %d", lcase[pre_lc].name, parameter, pre_lc+1);
  graph(buffer);
}



void selectEntity( int selection )
{
  int i, e[4];
#if TEST
  printf (" in selectEntity sequenceFlag:%d animFlag:%d\n",sequenceFlag, animFlag);
#endif 

  /* change the element and face mode to 'value' */
  for(i=0; i<anzGeo->psets; i++)
    if((pset[i].type[0]=='e')||(pset[i].type[0]=='f')) pset[i].type[1]='v';

  cur_entity=selection-1;
  cur_lc=pre_lc;
  v_scale=1.;
  if(scale->lock!='l') scale->smin=scale->smax=0;

  if(sequenceFlag)
  {
    /* if unselected seqLC[1] == 0 */
    /* if unselected seqLC[2] == anz->l */
    if(seqLC[1]<=seqLC[0]) seqLC[1]=seqLC[0]+1;
    seq_nlc=3;

    if((vectorFlag)&&(lcase[cur_lc].ictype[cur_entity]==2))
    {
      /* search all necessary entities */
      if (lcase[seqLC[0]].icind1[cur_entity]==1)      { e[0]=cur_entity+1; e[1]=cur_entity+2; e[2]=cur_entity+3; e[3]=-1;}         
      else if (lcase[seqLC[0]].icind1[cur_entity]==2) { e[0]=cur_entity; e[1]=cur_entity+1; e[2]=cur_entity+2; e[3]=-1;}         
      else if (lcase[seqLC[0]].icind1[cur_entity]==3) { e[0]=cur_entity-1; e[1]=cur_entity; e[2]=cur_entity+1; e[3]=-1;}         
      else if (lcase[seqLC[0]].icind1[cur_entity]==0) { e[0]=cur_entity-2; e[1]=cur_entity-1; e[2]=cur_entity; e[3]=cur_entity+1;}
      else
      {
        /* vector-components not located before the vector-value */
	printf("\n ERROR: entity is not part of a vector\n");
        return;
      }
      sprintf(buffer,"%d %d %d e", seqLC[0]+1, seqLC[1]+1, seqLC[2]+1);
      for(i=0; i<3; i++)
      {
        if((lcase[seqLC[0]].icind1[e[i]-1]==i+1)&&(lcase[seqLC[0]].ncomps>=e[i]))
          sprintf( &buffer[strlen(buffer)]," %d",e[i]);
        else
        {
          /* vector-components not located before the vector-value */
          printf("\n ERROR: entity is not part of a vector\n");
          return;
        }
      }
      if(e[3]>-1) sprintf( &buffer[strlen(buffer)]," %d",e[3]);
      selectData(buffer);
      return;
    }
    else
    {
      if((compare(lcase[seqLC[0]].name, "DISP", 4)==4)&&(cur_entity==3))
      {
        /* no fringe plot, just the animated deformation */
        sprintf(buffer,"%d %d %d a", seqLC[0]+1, seqLC[1]+1, seqLC[2]+1);
        selectData(buffer);
      }
      else
      {
        sprintf(buffer,"%d %d %d e", seqLC[0]+1, seqLC[1]+1, seqLC[2]+1);
        sprintf( &buffer[strlen(buffer)]," %d",cur_entity+1);
        selectData(buffer);
      }
      return;
    }
  }
  else
  {
    if((vectorFlag)&&(lcase[cur_lc].ictype[cur_entity]==2))
    {
      /* search all necessary entities */
      if (lcase[cur_lc].icind1[cur_entity]==1)      { e[0]=cur_entity+1; e[1]=cur_entity+2; e[2]=cur_entity+3; e[3]=-1;}
      else if (lcase[cur_lc].icind1[cur_entity]==2) { e[0]=cur_entity; e[1]=cur_entity+1; e[2]=cur_entity+2; e[3]=-1;}
      else if (lcase[cur_lc].icind1[cur_entity]==3) { e[0]=cur_entity-1; e[1]=cur_entity; e[2]=cur_entity+1; e[3]=-1;}
      else if (lcase[cur_lc].icind1[cur_entity]==0) { e[0]=cur_entity-2; e[1]=cur_entity-1; e[2]=cur_entity; e[3]=cur_entity+1;}
      else
      {
        /* components not located before the entity of the vector-length */
	printf("\n ERROR: entity is not part of a vector\n");
        return;
      }
      sprintf(buffer,"%d e",cur_lc+1 );
      for(i=0; i<3; i++)
      {
        if((lcase[cur_lc].icind1[e[i]-1]==i+1)&&(lcase[cur_lc].ncomps>=e[i]))
          sprintf( &buffer[strlen(buffer)]," %d",e[i]);
        else 
        {
          /* components not located before the entity of the vector-length */
          printf("\n ERROR: entity is not part of a vector\n");
          return;
	}
      }
      if(e[3]>-1) sprintf( &buffer[strlen(buffer)]," %d",e[3]);
      selectData(buffer);
      return;
    }
  }
  drawMode=1;
  if(!animFlag) ConfigureAndShowWindow_Load();
}



void selectEntityMenu( int selection )
{

#if TEST
  printf (" in selectEntityMenu\n");
#endif 
  animFlag=0;
  selectEntity(selection);
}



/* Dataset creation */
void createDatasetEntries(void)
{
  int  i,j,k, screen_height, menus, lc_per_menu, lc_per_basmenu, steps=1;
  static int  *mds=NULL;
  static int  *subsubmenu_load, *sub3menu_load;

  if(!inpformat) return;

  glutSetWindow( w0);
  if(!anz->l)
  {
    createNewMainMenu();
    glutSetWindow( activWindow);
    return;
  }

  /* check how much Step_numbers we have */
  if ( (mds = (int *)realloc((int *)mds, (steps+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed createDatasetEntries\n\n") ;
  mds[0]=0;
  for(i=1; i<anz->l; i++)
  {
    if(lcase[i].step_number!=lcase[i-1].step_number)
    {
      if ( (mds = (int *)realloc((int *)mds, (steps+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed createDatasetEntries\n\n") ;
      mds[steps]=i;
      steps++;
    }
  }
  //printf("ds:%d steps:%d\n", anz->l, steps);


  /* calculate how much lines of Steps can be displayed on the screen */
  screen_height=glutGet(GLUT_SCREEN_HEIGHT);
  if (!screen_height)
  { screen_height=800; printf(" screen not known, assume screen_height of 800 pixels\n"); } 
  lc_per_menu=screen_height/GLUT_MENU_POINT_HEIGHT ;

#if TEST
  printf ("lc_per_menu:%d \n",lc_per_menu);
#endif  

  if(steps>1)
  {
    glutDestroyMenu( submenu_load );
    submenu_load = glutCreateMenu( selectDataset );

    /* check if the remaining lc's fit in the remaining space of the basmenu */
    menus=steps/lc_per_menu;
    if( lc_per_menu-menus < steps%lc_per_menu ) menus++;
    lc_per_basmenu=steps - menus*lc_per_menu;

    /* define the Datasets */
    if ( (sub3menu_load = (int *)realloc((int *)sub3menu_load, (steps+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed createDatasetEntries\n\n") ;
    i=0;
    for (j=0; j<steps; j++ )
    {
      sub3menu_load[j] = glutCreateMenu( selectDataset );
      do
      {
        sprintf (buffer,"%d %.8s", i+1, lcase[i].name);
        glutAddMenuEntry( buffer, i);
        i++;
        if (i>=anz->l) break;
      }while(lcase[i-1].step_number==lcase[i].step_number);
    }
  
    /* define the additional submenus */
    if ( (subsubmenu_load = (int *)realloc((int *)subsubmenu_load, (steps+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed createDatasetEntries\n\n") ;
    i=lc_per_basmenu; if(i<0) i=0;
    for (j=0; j<menus; j++ )
    {
      subsubmenu_load[j] = glutCreateMenu( NULL );
      for (k=0; k<lc_per_menu; k++ )
      {
        if (i>=steps)
        {
          printf (" WARNING: To much Datasets for the menu. Please use the 'ds' command to access the Datasets %d to %d\n", mds[i], anz->l); 
          break;
	}
        //sprintf (buffer,"%d %.8s %lf %s", i+1, lcase[mds[i]].dataset_name, lcase[mds[i]].value, lcase[mds[i]].dataset_text );
        sprintf (buffer,"%d %.8s %lf %s", lcase[mds[i]].step_number, lcase[mds[i]].dataset_name, lcase[mds[i]].value, lcase[mds[i]].dataset_text );
        glutAddSubMenu( buffer, sub3menu_load[i] );
        i++;
      }
    }
  
    /* define the main-lc-menu */
    glutSetMenu( submenu_load );
    for (i=0; i<lc_per_basmenu; i++ )
    {
      //sprintf (buffer,"%d %.8s %lf %s", i+1, lcase[mds[i]].dataset_name, lcase[mds[i]].value, lcase[mds[i]].dataset_text );
      sprintf (buffer,"%d %.8s %lf %s", lcase[mds[i]].step_number, lcase[mds[i]].dataset_name, lcase[mds[i]].value, lcase[mds[i]].dataset_text );
      glutAddSubMenu( buffer, sub3menu_load[i] );
    }
    for (i=0; i<menus; i++) 
    {
      glutAddSubMenu( "-MORE- ", subsubmenu_load[i] );
      if(i>lc_per_menu) break;
    }
    glutAddSubMenu( "-Entity- ", subsubmenu_entity );

    selectDataset(cur_lc);
  }
  else
  {
    glutDestroyMenu( submenu_load );
    submenu_load = glutCreateMenu( selectDataset );

    /* check if the remaining lc's fit in the remaining space of the basmenu */
    menus=anz->l/lc_per_menu;
    if( lc_per_menu-menus < anz->l%lc_per_menu ) menus++;
    lc_per_basmenu=anz->l-menus*lc_per_menu;

#if TEST
    printf ("lc_per_menu:%d lc_per_basmenu:%d menus:%d\n",lc_per_menu, lc_per_basmenu, menus );
#endif  

    /* define the additional submenus */
    if ( (subsubmenu_load = (int *)realloc((int *)subsubmenu_load, (menus+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed createDatasetEntries\n\n") ;
    i=lc_per_basmenu; if(i<0) i=0;
    for (j=0; j<menus; j++ )
    {
      subsubmenu_load[j] = glutCreateMenu( selectDataset );
      for (k=0; k<lc_per_menu; k++ )
      {
        if (i>=anz->l) break;
        sprintf (buffer,"%d %.8s %lf %s", i+1, lcase[i].name, lcase[i].value, lcase[i].dataset_text );
        glutAddMenuEntry( buffer, i);
        i++;
      }
    }
  
    /* define the main-lc-menu */
    glutSetMenu( submenu_load );
    for (i=0; i<lc_per_basmenu; i++ )
    {
        sprintf (buffer,"%d %.8s %lf %s", i+1, lcase[i].name, lcase[i].value, lcase[i].dataset_text );
        glutAddMenuEntry( buffer, i);
    }
    for (i=0; i<menus; i++) 
    {
        glutAddSubMenu( "-MORE- ", subsubmenu_load[i] );
        if(i>lc_per_menu) break;
    }
    glutAddSubMenu( "-Entity- ", subsubmenu_entity );

    selectDataset(cur_lc);
  }
  createNewMainMenu();
  glutSetWindow( activWindow);
}


/* selection is the menu-nr of the selected dataset (lc), starting with 0 */
void selectDataset( int selection )
{
  pre_lc= selection;

  /* check if the data of the specified lcase (Dataset) are already available */
  if (!lcase[pre_lc].loaded)
  {
    if( pre_readfrdblock(copiedNodeSets , pre_lc, anz, node, lcase )==-1) 
    {
      printf("ERROR in selectDataset: Could not read data for Dataset:%d\n", pre_lc+1); 
      return;
    }
    calcDatasets( pre_lc, anz, node, lcase );
  }

  if(sequenceFlag)
  {
    if (seq_nlc>2) {seq_nlc=0; seqLC[1]=0; seqLC[2]=anz->l-1; }
    seqLC[seq_nlc]=pre_lc;
    if ((seq_nlc>0)&&(compareStrings(lcase[seqLC[0]].name,lcase[seqLC[seq_nlc]].name)<1))
    {
      errMsg(" WARNING: selected Dataset %s is of different name than the 1st selected %s\n", lcase[seqLC[seq_nlc]].name,lcase[seqLC[0]].name);
    }
    if (seq_nlc>1)
    {
      printf(" selection finished, select entity\n");
    }
    seq_nlc++;
  }
  else {
      seq_nlc=0;
      seqLC[1]=0;
      seqLC[2]=anz->l-1;
  }/* no sequence until a new entity is selected and all data are prepared */

  recompileEntitiesInMenu(pre_lc);
}


void createNewMainMenu(void)
{
  if(!inpformat) return;

  glutSetWindow( w0);
  //if (mainmenu>-1) glutDestroyMenu( mainmenu );
    mainmenu = glutCreateMenu( menu );
    if(anz->l>0) glutAddSubMenu  ( "Datasets   ", submenu_load );
    glutAddSubMenu  ( "Viewing     ", submenu_view );
    glutAddSubMenu  ( "Animate     ", submenu_animate );
    glutAddMenuEntry( "Frame       ", 1);
    glutAddMenuEntry( "Zoom        ", 2);
    glutAddMenuEntry( "Center      ", 3);
    glutAddMenuEntry( "Enquire     ", 4);
    glutAddSubMenu  ( "Cut         ", submenu_cut  );
    glutAddSubMenu  ( "Graph       ", submenu_graph);
    glutAddSubMenu  ( "Orientation ", submenu_orientation );
    glutAddSubMenu  ( "Hardcopy    ", submenu_hardcopy);
    glutAddSubMenu  ( "Help        ", submenu_help);
    glutAddMenuEntry( " -QUIT-     ", 5);
    glutAttachMenu(GLUT_LEFT_BUTTON);
  glutSetWindow( activWindow);
}



void recompileEntitiesInMenu(int lc)
{
  int i, entries;

  if(!inpformat) return;

  glutSetMenu( submenu_load );
  if(lcase[lc].npheader)
  {
    glutSetMenu( subsubmenu_parameter );
    entries=glutGet(GLUT_MENU_NUM_ITEMS);
    for (i=entries; i>0; i--) glutRemoveMenuItem(i);
    for (i=1; i<=lcase[lc].npheader; i++)
    {
      sprintf (buffer,"%d %s", i, &lcase[lc].pheader[i-1][6] );
      glutAddMenuEntry( buffer, i);
    }
  }

  glutSetMenu( subsubmenu_entity );
  entries=glutGet(GLUT_MENU_NUM_ITEMS);
  for (i=entries; i>0; i--) glutRemoveMenuItem(i);

  for (i=1; i<=lcase[lc].ncomps; i++)
  {
    if(lcase[lc].menu[i-1])
    {
      sprintf (buffer,"%d %s", i, lcase[lc].compName[i-1] );
      glutAddMenuEntry( buffer, i);
    }
  }
  if(lcase[lc].npheader)
    glutAddSubMenu( "-Parameter- ", subsubmenu_parameter );
}



void addDispToCoordinates( Nodes *node_local)
{
  int i,j,k,lc, foundDisp=0, compareChars;
  static Nodes *n_disp=NULL, *orig_nodes;
  static char buffer[2][MAX_LINE_LENGTH];
  static Summen orig[1];
  double amplitude;

  if(!addDispFlag)
  {
    strcpy(buffer[0],"DISP ");
    strcpy(buffer[1],"DISPI");
    lc=pre_lc;
    /* if the selected lc is not a disp lc, search a related disp lc */
    if((compare(lcase[lc].name, "DISP", 4)==4)&&(lcase[lc].ictype[0]!= 12)) foundDisp=1;
    else
    {
      /* since real and imaginary part use different names since ccx_2.9 it is necessary to compare the 
         names only for the length excluding the last char if its a 'I' */
      compareChars=strlen(lcase[lc].name)-1;
      for(k=compareChars;k>0; k--) if(lcase[lc].name[k]!=' ') break;
      compareChars=k+1;
      if(lcase[lc].name[compareChars-1]=='I') j=1; else j=0;;

      if(lc) { for (i=lc-1; i>=0; i--) { if(lcase[i].step_number!=lcase[lc].step_number) break; } i++; }
      else i=1;
      while((i<anz->l)&&(lcase[i].step_number==lcase[lc].step_number))
      {
        if((compare(lcase[i].name, buffer[j], 5)==5)&&(lcase[i].ictype[0]!= 12))
        {
	  //printf("lcase[i].name:%s lcase[lc].name:%s compareChars:%d\n", lcase[i].name,lcase[lc].name,compareChars);
          lc=i;
          foundDisp=1;

          /* check if the data of the specified lcase (Dataset) are already available */
          if (!lcase[lc].loaded)
          {
            if( pre_readfrdblock(copiedNodeSets , lc, anz, node_local, lcase )==-1) 
            {
              printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
              return;
            }
            calcDatasets( lc, anz, node_local, lcase );
          }
          break;
	}
        i++;
      }
    }
    if (foundDisp)
    {
      /* add displacements to the node-coords and return a pointer to the changed nodes */

      /* create new nodes */
      if ( (n_disp = (Nodes *)realloc((Nodes *)n_disp, (anz->nmax+1) * sizeof(Nodes))) == NULL )
        printf("\n\n ERROR: malloc failed n_disp\n");
      else
        if(printFlag) printf (" n_disp allocated \n");
      amplitude= anim_faktor;

      /* check if the data of the specified lcase (Dataset) are already available */
      if (!lcase[lc].loaded)
      {
        if( pre_readfrdblock(copiedNodeSets , lc, anz, node_local, lcase )==-1) 
        {
          printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
          scalNodes ( anz->n, node_local, scale );
          return;
        }
        calcDatasets( lc, anz, node_local, lcase );
        recompileEntitiesInMenu(lc);
      }
      descalNodes ( anz->n, node_local, scale );
      addDispToNodes(anz, n_disp, node_local, e_enqire, lcase, lc, amplitude);
      scalNodes ( anz->n, n_disp, scale );

      addDispFlag=!addDispFlag;
      orig_nodes=node_local;
      orig->orign=    anz->orign;	  
      orig->n=	  anz->n;	  
      orig->e=	  anz->e;	  
      orig->f=	  anz->f;	  
      orig->g=	  anz->g;	  
      orig->t=	  anz->t;	  
      orig->l=	  anz->l;	  
      orig->olc=	  anz->olc;	  
      orig->orignmax= anz->orignmax;  
      orig->nmax=	  anz->nmax;	  
      orig->nmin=	  anz->nmin;	  
      orig->emax=	  anz->emax;	  
      orig->emin=	  anz->emin;	  
      orig->sets=	  anz->sets;	  
      orig->mats=	  anz->mats;	  
      orig->amps=	  anz->amps;	  
      orig->noffs=	  anz->noffs;	  
      orig->eoffs=	  anz->eoffs;     

      /* update node */
      node=n_disp;
      getFaceNormalen( face, node, anz );
      getElemNormalen( e_enqire, node, anz->e );
      updateDispLists(); 
      return;
    }
    else
    {
      errMsg ("\n ERROR: choose a Dataset with displacements before\n");
      printf ("\n  Remark: Before you use this option select the Dataset and the Entity you want to display and then the related Dataset of the displacements (but no entity).\n");
    }
  }
  else
  {
    addDispFlag=!addDispFlag;
    /* return the original pointer to the nodes  */
    descalNodes ( anz->n, node_local, scale );
    free(node_local);
    n_disp=NULL;

    /* update node */
    node=orig_nodes;
    anz->orign=   orig->orign;   
    anz->n=	orig->n;	 
    //anz->e=	orig->e;	 
    //anz->f=	orig->f;	 
    //anz->g=	orig->g;	 
    //anz->t=	orig->t;	 
    //anz->l=	orig->l;	 
    //anz->olc=	orig->olc;	 
    anz->orignmax=orig->orignmax;
    anz->nmax=	orig->nmax;	 
    anz->nmin=	orig->nmin;	 
    //anz->emax=	orig->emax;	 
    //anz->emin=	orig->emin;	 
    //anz->sets=	orig->sets;	 
    //anz->mats=	orig->mats;	 
    //anz->amps=	orig->amps;	 
    //anz->noffs=	orig->noffs;	 
    //anz->eoffs=	orig->eoffs;	 
    scalNodes ( anz->n, node, scale );
    getFaceNormalen( face, node, anz );
    getElemNormalen( e_enqire, node, anz->e );
    updateDispLists(); 
    return;
  }
}



void selectView( int selection )
{
  int i;
  static GLint ipuf[2];
  static char  mem_elemEdgeFlag, mem_surfFlag;
  char addDispFlagLocal=0;

  glutSetWindow( w1);
  switch (selection) {
  case 1:
      if(getSetNr("-qcut")) 
      {
        if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
        zap("-qcut");
        if(addDispFlagLocal==2) { addDispToCoordinates(node); }
      }
      vectorFlag=0;
      sequenceFlag=0;
      animFlag=0;
      drawMode=2;
      ConfigureAndShowWindow_Light();
    break;
  case 2:
    /* show bad elements */
    i=calcBadElements("all");
    if (i>0)
    {
      printf("found %d bad elements in set:all (stored in set:%s)\n", i, specialset->njby);
      sprintf(buffer, "e %s i",specialset->njby);
      plot(buffer);
    }
    else
    {
      printf("No bad elements in set:all\n");
    }
    if(drawMode==1) ConfigureAndShowWindow_Load();
    else if(drawMode<4) ConfigureAndShowWindow_Light();
    break;
  case 3:
    glPointSize (1);
    glGetIntegerv( GL_POLYGON_MODE, ipuf );
    if ( ipuf[1] != GL_FILL )
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    break;
  case 4:
    glPointSize (1);
    glGetIntegerv( GL_POLYGON_MODE, ipuf );
    if ( ipuf[1] != GL_LINE )
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
    break;
  case 5:
    glPointSize (1);
    glGetIntegerv( GL_POLYGON_MODE, ipuf );
    if ( ipuf[1] != GL_POINT )
      glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
    break;
  case 6:
    glGetIntegerv( GL_CULL_FACE_MODE, ipuf );
    if ( ipuf[0] == GL_BACK )
      {
      glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
      glCullFace ( GL_FRONT );
      }
    else
      {
      glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
      glCullFace ( GL_BACK );
      }
    break;
  case 7:
      modelEdgeFlag=!modelEdgeFlag;
      if (modelEdgeFlag) /* copy from updateDispLists() */
        drawModelEdges( list_model_edges, basCol[0], 2., anz->g, node );
    break;
  case 8:
      elemEdgeFlag=!elemEdgeFlag;
      if (elemEdgeFlag)
      {
        if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
        else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );
      }
    break;
  case 9:
      /* change between element and face mode */
      surfFlag=!surfFlag;
      if(drawMode!=4)
      {
        if(surfFlag)
        {
          for(i=0; i<anzGeo->psets; i++ ) if(pset[i].type[0]=='e') pset[i].type[0]='f';
        }
        else
        {
          for(i=0; i<anzGeo->psets; i++ ) if(pset[i].type[0]=='f') pset[i].type[0]='e';
        }
      }
      redraw();
    break;
  case 10:
      movezFlag=!movezFlag;
      if( movezFlag)
      {
        glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
        mem_surfFlag=surfFlag;
        mem_elemEdgeFlag=elemEdgeFlag;
        surfFlag=0;
        elemEdgeFlag=1;
        printf ("press middle Mouse Button to move in Z Direction\n");
        if(drawMode!=4)
        {
          if(mem_surfFlag==1)
          { 
            for(i=0; i<anzGeo->psets; i++ ) if(pset[i].type[0]=='f') pset[i].type[0]='e';
          }
        }
      }
      else
      {
        surfFlag=mem_surfFlag;
        elemEdgeFlag=mem_elemEdgeFlag;
        glGetIntegerv( GL_CULL_FACE_MODE, ipuf );
        if ( ipuf[0] == GL_BACK )
        {
          glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
        }
        else
        {
          glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
        }
        printf ("press middle Mouse Button to Zoom in and out\n");
        if(drawMode!=4)
        {
          if(mem_surfFlag==1)
          { 
            for(i=0; i<anzGeo->psets; i++ ) if(pset[i].type[0]=='e') pset[i].type[0]='f';
          }
        }
      }
      askGLError("movezFlag, GL_CULL_FACE");
      redraw();
    break;
  case 11:
    if(foregrndcol) { foregrndcol=0; foregrndcol_rgb[0]=foregrndcol_rgb[1]=foregrndcol_rgb[2]=foregrndcol_rgb[3]=0.; }
    else            { foregrndcol=1; foregrndcol_rgb[0]=foregrndcol_rgb[1]=foregrndcol_rgb[2]=foregrndcol_rgb[3]=1.; }
    if(backgrndcol) { backgrndcol=0; backgrndcol_rgb[0]=backgrndcol_rgb[1]=backgrndcol_rgb[2]=backgrndcol_rgb[3]=0.; }
    else            { backgrndcol=1; backgrndcol_rgb[0]=backgrndcol_rgb[1]=backgrndcol_rgb[2]=backgrndcol_rgb[3]=1.; }
    for (i=0; i<anzGeo->psets; i++ )
    {
      if(pset[i].col==0) pset[i].col=1;
      else if(pset[i].col==1) pset[i].col=0;
    }
    updateDispLists();
    break;
  case 12:
    /* create a vector-plot */
    /* if one entity is choosen, check if it belongs to a vector (icind) */
    /* and choose also the other components */
    if(anz->l>0)
    {
      vectorFlag=!vectorFlag;
      if(vectorFlag)
      {
        entity_buf=cur_entity;
        selectEntity(cur_entity+1 );
        printf("\n Vector-plot selected. Works only for data marked internally as a vector. In other cases use the 'ds' command.\n");
      }
      else
      {
        cur_entity=entity_buf;
        selectEntity(cur_entity+1 );
        printf("\n Vector-plot off\n");
      }
    }
   break;
  case 13:
    /* add/remove the actual displacements to all node-coordinates multiplied by anim_faktor */
    if(!sequenceFlag)
    {
      addDispToCoordinates(node);
      if(addDispFlag) printf("\n displacements related to dataset:%d added. Scale them with 'scal d <value>'\n",pre_lc+1);
      else printf("\n displacements removed\n");  
      redraw();
    }
    else printf("\n ERROR: 'Toggle Add-Displacement' can not be selected during animations.\n");
    break;
  }
  glutSetWindow( w0);
  glutPostRedisplay();
}


void pre_animate(char *string)
{
  char type[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH];

  if(!inpformat) return;
  param[0]=0;
  sscanf(string, "%s %s", type, param );

  glutSetWindow( w1);
  if (compare(type, "tune", 2)==2)
  {
    anim_faktor=atof(param);
    /* remove the displacements and use the scaled ones */
    if(addDispFlag==1)
    {
      addDispToCoordinates(node);
      addDispToCoordinates(node);
    }
  }
  else if (compare(type, "start", 3)==3) { changeAnimation(1); }
  else if (compare(type, "steps", 3)==3) anim_steps=atof(param);
  else if (compare(type, "time", 2)==2) time_per_period=atof(param)*1000.;
  else if (compare(type, "real", 2)==2)
  {
    if (compare(param, "off", 2)==2) halfperiod=0;
    else halfperiod=1;
  }
  else if (compare(type, "model", 2)==2)
  {
    if (compare(param, "off", 2)==2) modelEdgeFlag_Static=0;
    else modelEdgeFlag_Static=1;
    if (modelEdgeFlag_Static) /* copy from updateDispLists() */
      drawModelEdges( list_model_edges, basCol[0], 2., anz->g, node );
  }
  else if (compare(type, "elem", 2)==2)
  {
    if (compare(param, "off", 2)==2) elemEdgeFlag_Static=0;
    else elemEdgeFlag_Static=1;
    if (elemEdgeFlag_Static)
    {
      if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
      else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );
    }
  } 
  redraw();
}


void pre_view(char *string)
{
  int i, length;
  char type[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH];
  GLint ipuf[2];
  char addDispFlagLocal=0;

  param[0]=0;
  length=sscanf(string, "%s %s", type, param );
  if (compare(type, "disp", 3)==3)
  {
    if(addDispFlag==1)
    {
      if(length==2) if(compare(param, "off", 2)==2)
      {
        addDispToCoordinates(node);
      }
    }
    else
    {
      if((length==2)&&(compare(param, "off", 2)==2));
      else
      {
        addDispToCoordinates(node);
        printf("\n displacements of dataset:%d added. Scale them with 'scal d <value>'\n",pre_lc+1); 
      }
    }
  }
  if(!inpformat) return;

  glutSetWindow( w1);
  if (compare(type, "fill", 2)==2)
  {
    glPointSize (1);
    glGetIntegerv( GL_POLYGON_MODE, ipuf );
    if ( ipuf[1] != GL_FILL )
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
  else if (compare(type, "line", 2)==2)
  {
    glPointSize (1);
    glGetIntegerv( GL_POLYGON_MODE, ipuf );
    if ( ipuf[1] != GL_LINE )
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  }
  else if (compare(type, "point", 2)==2)
  {
    glPointSize (1);
    glGetIntegerv( GL_POLYGON_MODE, ipuf );
    if ( ipuf[1] != GL_POINT )
      glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
  }
  else if (compare(type, "bg", 2)==2)
  {
    if(length==2)
    {
      if (compare(param, "k", 1)==1) { foregrndcol=0; backgrndcol=1; }
      else if (compare(param, "w", 1)==1) { foregrndcol=1; backgrndcol=0; }
    }
    if(foregrndcol) { foregrndcol=0; foregrndcol_rgb[0]=foregrndcol_rgb[1]=foregrndcol_rgb[2]=foregrndcol_rgb[3]=0.; }
    else            { foregrndcol=1; foregrndcol_rgb[0]=foregrndcol_rgb[1]=foregrndcol_rgb[2]=foregrndcol_rgb[3]=1.; }
    if(backgrndcol) { backgrndcol=0; backgrndcol_rgb[0]=backgrndcol_rgb[1]=backgrndcol_rgb[2]=backgrndcol_rgb[3]=0.; }
    else            { backgrndcol=1; backgrndcol_rgb[0]=backgrndcol_rgb[1]=backgrndcol_rgb[2]=backgrndcol_rgb[3]=1.; }
    for (i=0; i<anzGeo->psets; i++ )
    {
      if(pset[i].col==0) pset[i].col=1;
      else if(pset[i].col==1) pset[i].col=0;
    }
    updateDispLists();
  }
  else if (compare(type, "edge", 2)==2)
  {
    modelEdgeFlag=1;
    if(length==2) { if (compare(param, "off", 2)==2) modelEdgeFlag=0; }
    if (modelEdgeFlag)
        drawModelEdges( list_model_edges, basCol[0], 2., anz->g, node );
  }
  else if (compare(type, "elem", 2)==2)
  {
    elemEdgeFlag=1;
    if(length==2) { if (compare(param, "off", 2)==2) elemEdgeFlag=0; }
    if (elemEdgeFlag)
    {
      if (surfFlag) drawDispListEdges(list_surf_edges, basCol[0], 1., 'f', node );
      else          drawDispListEdges(list_elem_edges, basCol[0], 1., 'e', node );
    }
  }
  else if (compare(type, "surf", 2)==2)
  {
      if(getSetNr("-qcut")) 
      {
        if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
        zap("-qcut");
        if(addDispFlagLocal==2) { addDispToCoordinates(node); }
      }
      surfFlag=1;
      sequenceFlag=0;
      animFlag=0;
      drawMode=2;
      for(i=0; i<anzGeo->psets; i++ ) if(pset[i].type[0]=='e') pset[i].type[0]='f';
      ConfigureAndShowWindow_Light();
  }
  else if (compare(type, "volu", 2)==2)
  {
      if(getSetNr("-qcut")) 
      {
        if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
        zap("-qcut");
        if(addDispFlagLocal==2) { addDispToCoordinates(node); }
      }
      surfFlag=0;
      sequenceFlag=0;
      animFlag=0;
      drawMode=2;
      for(i=0; i<anzGeo->psets; i++ ) if(pset[i].type[0]=='f') pset[i].type[0]='e';
      ConfigureAndShowWindow_Light();
  }
  else if (compare(type, "front", 3)==3)
  {
      glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
      glCullFace ( GL_BACK );
  }
  else if (compare(type, "back", 3)==3)
  {
      glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
      glCullFace ( GL_FRONT );
  }
  else if (compare(type, "vector", 3)==3)
  {
    if(anz->l>0)
    {
      vectorFlag=1;  
      if(length==2) { if (compare(param, "off", 2)==2) vectorFlag=0; }
      if(vectorFlag)
      {
        entity_buf=cur_entity;
        selectEntity(cur_entity+1 );
        printf("\n Vector-plot selected. Works only for data marked internally as a vector. In other cases use the 'ds' command.\n");
      }
      else
      {
        cur_entity=entity_buf;
        selectEntity(cur_entity+1 );
        printf("\n Vector-plot off\n");
      }
    }
  }
  glutPostRedisplay();
  glutSetWindow( w2 );
  glutPostRedisplay();
  glutSetWindow( w0);
  glutPostRedisplay();
}



void scaldataset(int lc, double factor)
{
  register int i,n;
  for(i=0; i<lcase[lc].ncomps; i++)
  {
    for(n=0; n<anz->n; n++)
    {
      if(node[node[n].nr].pflag==-1) continue;
      lcase[lc].dat[i][node[n].nr]*=factor;
    }
    lcase[lc].max[i]*=factor;
    lcase[lc].min[i]*=factor;
  }
  scale->smin*=factor;
  scale->smax*=factor;
}



void offsetdataset(int lc, double offset, int e)
{
  register int i,n;
  if(e==0)
  {
   for(i=0; i<lcase[lc].ncomps; i++)
   {
    for(n=0; n<anz->n; n++)
    {
      if(node[node[n].nr].pflag==-1) continue;
      lcase[lc].dat[i][node[n].nr]+=offset;
    }
    lcase[lc].max[i]+=offset;
    lcase[lc].min[i]+=offset;
   }
  }
  else
  {
    e--;
    for(n=0; n<anz->n; n++)
    {
      if(node[node[n].nr].pflag==-1) continue;
      lcase[lc].dat[e][node[n].nr]+=offset;
    }
    lcase[lc].max[e]+=offset;
    lcase[lc].min[e]+=offset;
  }
  if(cur_entity==e)
  {
    scale->smin+=offset;
    scale->smax+=offset;
  }
}



void selectData( char *record)
{
  int i,j, dim, length, lc[3], e[4], nlc, ne=0;
  char data[8][MAX_LINE_LENGTH], historyFlag=0;
  double factor=1., offset=0.;
  char parameter[20][MAX_LINE_LENGTH];

#if TEST
  printf (" in selectData\n");
#endif 

  /* change the element and face mode to 'value' */
  for(i=0; i<anzGeo->psets; i++)
    if((pset[i].type[0]=='e')||(pset[i].type[0]=='f')) pset[i].type[1]='v';

  //printf("record %s\n",record);
  length = sscanf( record,"%s %s %s %s %s %s %s %s", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

  if(anz->l<1)
  {
    printf(" ERROR: no Datasets available\n");
    strcpy(parameter[0], "ERROR: no Datasets available");
    write2stack(1, parameter);
    return;
  }

  /* search for 'e', after 'e' appear the entities (up to 3 for a vector-plot) */
  dim=nlc=ne=e[0]=0;
  for(i=0; i<length; i++)
  {
    if(dim) e[ne++]=atoi(data[i]);
    else if(data[i][0]=='a') { e[ne++]=data[i][0]; dim=1; if(data[i][1]=='h') historyFlag=1; if(length>i+1) e[ne++]=atoi(data[i+1]); break; }
    else if(data[i][0]=='s') { factor=atof(data[i+1]); dim=-1; break; }
    else if(data[i][0]=='o') { offset=atof(data[i+1]); dim=-2; if(length>i+2) e[ne++]=atoi(data[i+2]); break; }
    else if(data[i][0]=='e') { dim=length-i-1; if(data[i][1]=='h') historyFlag=1; }
    else if(!dim)
    {
      if(data[i][0]=='l') lc[nlc++]=anz->l;
      else if(data[i][0]=='-') lc[nlc++]=anz->l+atoi(data[i]); 
      else lc[nlc++]=atoi(data[i]);
    }
  }
  /* check the selection */
  for(i=0; i<nlc; i++)
  {

    if( lc[i]<1)
    {
      errMsg ("ERROR: Dataset %d not known, min Dataset %d\n", lc[i], 1 );
      sprintf(parameter[0], "ERROR: Dataset %d not known, min Dataset %d\n", lc[i], 1 );
      write2stack(1, parameter);
      return;
    }

    if( lc[i]>anz->l)
    {
      errMsg ("ERROR: Dataset %d not known, max Dataset %d\n", lc[i], anz->l );
      sprintf(parameter[0], "ERROR: Dataset %d not known, max Dataset %d\n", lc[i], anz->l );
      write2stack(1, parameter);
      return;
    }

    /* check if the data of the specified lcase (Dataset) are already available */
    if (!lcase[lc[i]-1].loaded)
    {
      if( pre_readfrdblock(copiedNodeSets , lc[i]-1, anz, node, lcase )==-1) 
      {
        printf("ERROR in selectData: Could not read data for Dataset:%d\n", lc[i]); 
        return;
      }
      calcDatasets( lc[i]-1, anz, node, lcase );
    }
  }

  for(j=0; j<ne; j++)
  {
    if ( ( e[j]=='a' ) || ( (e[j]<=lcase[lc[0]-1].ncomps) && (e[j]>0) ) ) ;
    else { errMsg("ERROR: entity not known %d\n", e[j]); return; }
  }

  /* if datasets are to be scaled */
  if(dim<0)
  {
    if(nlc==1)
    {
      if(dim==-1) scaldataset(lc[0]-1,factor);
      if(dim==-2) offsetdataset(lc[0]-1,offset, e[0]);
    } 
    else
    { 
      for(i=0; i<nlc; i++)
      {
        if((i>0)&&(compareStrings(lcase[lc[0]-1].name,lcase[lc[i]-1].name)<1))
        {
          errMsg(" WARNING: selected Dataset %s is of different name than the 1st selected %s\n", lcase[lc[i]-1].name, lcase[lc[0]-1].name);
          return;
        }
      }
      if(dim==-1) { for(i=lc[0]-1; i<lc[2]; i+=lc[1]-lc[0]) scaldataset(i,factor); }
      if(dim==-2) { for(i=lc[0]-1; i<lc[2]; i+=lc[1]-lc[0]) offsetdataset(i,offset, e[0]); }
    } 
    ConfigureAndShowWindow_Load();
    return;
  } 
 
  /* no entity selected, just a single one or a sequence */
  if(!dim)
  {
    dim=1;
    if(nlc==1)
    {
      drawMode=1;
      selectDataset(lc[0]-1);
    }
    /* select a sequence of Datasets */
    else
    {
      sequenceFlag=1;
      seq_nlc=nlc;
      for(i=0; i<seq_nlc; i++)
      {
        if((i>0)&&(compareStrings(lcase[lc[0]-1].name,lcase[lc[i]-1].name)<1))
        {
          errMsg(" WARNING: selected Dataset %s is of different name than the 1st selected %s\n", lcase[lc[i]-1].name, lcase[lc[0]-1].name);
        }
        else seqLC[i]=lc[i]-1;
      }
      createDsSequence(seq_nlc, seqLC);
    }
    return;
  } 

  /* if nlc > 1 generate a sequence */
  /* if dim > 1 generate a vector plot */
  if((dim>1)&&(e[0]!='a')) vectorFlag=1; else vectorFlag=0;
  if(nlc>1) historyFlag=1;
  if(!historyFlag)
  { 
    sequenceFlag=0;
    v_dim=0;
    selectDataset(lc[0]-1);
    if( e[0]=='a' )
    {
      animFlag=1;
      if(ne>1)
      {
        drawMode=1;
        selectEntity(e[1]);
        if(inpformat)
        {
          glutSetWindow( w1 );
          //glShadeModel ( GL_SMOOTH );
#if INX_MODE
          defineColIndexes_load();
#endif
#if TEX_MODE
          defineColTextur_load();
#endif
        }
        if (lcase[cur_lc].irtype == 3) /* element data */
        {
          elementDataset( cur_entity, cur_lc, anz, scale, lcase, offset, maxIndex, steps );
        }
        else
        {    
#if INX_MODE
          nodalDataset( cur_entity, cur_lc, anz, scale, node, lcase, colNr, offset, maxIndex, steps, 1 );
#endif 
#if TEX_MODE
          nodalDataset( cur_entity, cur_lc, anz, scale, node, lcase, colNr, 1 );
#endif
        }
      }
      else
      {
        drawMode=2;
      }
      ConfigureAndShowWindow_Light();
    }

    /* prepare a vector-plot */
    else if((ne>1)&&(e[0]!='a'))
    {
      animFlag=0;
      cur_entity=e[ne-1]-1;
      drawMode=5;
      v_dim=ne;
      seq_nlc=3;
      seqLC[0]=lc[0]-1;
      seqLC[1]=lc[0]-1;
      seqLC[2]=lc[0]-1;
      for(i=0; i<ne; i++) entity_v[i]=e[i]-1;
      if(scale->lock!='l') scale->smin=scale->smax=0;
      createDsSequence(seq_nlc, seqLC);
      ConfigureAndShowWindow_Vector();
      return;
    }
    /* prepare a scalar plot */
    else
    {
      animFlag=0;
      drawMode=1;
      selectEntity(e[0]);
    }
  }
  else // prepare a sequence
  {
    /* store the sequence of Datasets */
    sequenceFlag=1;
    seq_nlc=nlc;
    seqLC[1]=0;
    for(i=0; i<seq_nlc; i++)
    {
      seqLC[i]=lc[i]-1;
    }
    if(seqLC[1]<=seqLC[0]) { seqLC[1]=seqLC[0]+1; seq_nlc++; }
    createDsSequence(seq_nlc, seqLC);

    /* prepare an vector-plot */
    if((ne>1)&&(e[0]!='a'))
    {
      cur_entity=e[ne-1]-1;
      drawMode=5;
      v_dim=ne;
      for(i=0; i<ne; i++) entity_v[i]=e[i]-1;
      if(scale->lock!='l') scale->smin=scale->smax=0;
      ConfigureAndShowWindow_Vector();
    }
    else
    {
      if( e[0]=='a' )
      {
        if(ne>1)
        {
          drawMode=1;
          cur_entity=e[1]-1;
        }
        else
        {
          cur_entity=lcase[lc[0]-1].ncomps-1;
          drawMode=2;
          /* change the color to "illuminated" */
          for (j=0; j<anzGeo->psets; j++ )
          {
            if((pset[j].type[0]=='e')||(pset[j].type[0]=='f'))
              pset[j].type[1]=0;
          }
	}
        ConfigureAndShowWindow_Light();
      }
      else
      {
        drawMode=1;
        cur_entity=e[0]-1;
        ConfigureAndShowWindow_Load();
      }
    }
  }
}



/* keyboard history */
void specialKeyboard( int gkey, int x, int y )
{
  char  echo;
  int i, j, pos, setNr, lc;
  static char  prognam[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH], lastset[MAX_LINE_LENGTH], col[MAX_LINE_LENGTH];
  static char buffer[MAX_LINE_LENGTH], keystroke2[MAX_LINE_LENGTH];

  if((gkey==GLUT_KEY_UP)||(gkey==GLUT_KEY_DOWN))
  {
    for(i=0; i<strlen(keystroke); i++)
    {
      /* go left */
      echo=( char )0xff08;
      putchar(echo);
    }
    for(i=0; i<strlen(keystroke); i++)
    {
      /* overwrite old command */
      echo=' ';
      putchar(echo);
    }
    for(i=0; i<strlen(keystroke); i++)
    {
      /* go left */
      echo=( char )0xff08;
      putchar(echo);
    }
  
    if(gkey==GLUT_KEY_UP)
    {
      key_pointer--;
    }
    if(gkey==GLUT_KEY_DOWN)
    {
      key_pointer++;
    }
    if (key_pointer<0) key_pointer=0;
    if (key_pointer>=nkey_history) key_pointer=nkey_history-1;
    for (i=0; i<MAX_LINE_LENGTH; i++) keystroke[i]='\0';
    if(key_history!=NULL) strcpy(keystroke,key_history[key_pointer]); 
    curshft=0;
    printf("%s",keystroke);
    fflush(stdout);
  }
  else if((gkey==GLUT_KEY_RIGHT)||(gkey==GLUT_KEY_LEFT))
  {
    if (gkey==GLUT_KEY_LEFT)
    {
      /* go left */
      //echo=( char )0xff08;
      curshft--;
      if(curshft < (-strlen(keystroke)) ) curshft=-strlen(keystroke);
      else 
      {
        echo=( char )8;
        putchar(echo);
      }
    }
    if (gkey==GLUT_KEY_RIGHT)
    {
      /* go right */
      //echo=( char )0xff0b;
      curshft++;
      if(curshft > 0)
      {
        curshft=0;
      }
      else 
      {
        echo=( char )keystroke[strlen(keystroke)-1+curshft];
        putchar(echo);
      }
    }
    fflush(stdout);  
  }

  else if((gkey==GLUT_KEY_PAGE_UP)||(gkey==GLUT_KEY_PAGE_DOWN))
  {
    /* the purpose is to remove the last displayed set and to display the next set */
    /* or if the last command was no plot/plus to display the next loadcase */

    if(nkey_history)
    {
      pos = sword( key_history[nkey_history-1], prognam);
      for(j=0;j<strlen(prognam); j++) prognam[j]=toupper(prognam[j]);

      if (( compare(prognam, "PLUS", 4) == 4 )||( compare(prognam, "PLOT", 4) == 4 ))
      {
        strcpy(keystroke2,key_history[nkey_history-1]);
        generateSetIndexes();

        col[0]='\0';
        sscanf(keystroke2, "%*s %s %s %s", type, lastset, col);
        setNr=getSetNr(lastset);
    
        if(gkey==GLUT_KEY_PAGE_UP)
        {
          for(i=setNr-1; i>=0; i--)
          {
            if( ((set[i].name != (char *)NULL )&&(set[i].type==0))&&
    	  ( ( (type[0]=='n')&& (set[i].anz_n) ) ||
                ( (type[0]=='e')&& (set[i].anz_e) ) || 
                ( (type[0]=='f')&& (set[i].anz_f) ) ||  
                ( (type[0]=='p')&& (set[i].anz_p) ) ||  
                ( (type[0]=='l')&& (set[i].anz_l) ) ||  
                ( (type[0]=='s')&& (set[i].anz_s) ) ||  
                ( (type[0]=='b')&& (set[i].anz_b) ) ||  
    	    ( (type[0]=='L')&& (set[i].anz_nurl) ) ||
    	    ( (type[0]=='S')&& (set[i].anz_nurs) ) ))  break;
          }
          if(i>=0) setNr=i;
        }
        if(gkey==GLUT_KEY_PAGE_DOWN)
        {
          for(i=setNr+1; i<anz->sets; i++)
          {
            if( ((set[i].name != (char *)NULL )&&(set[i].type==0))&&
    	  ( ( (type[0]=='n')&& (set[i].anz_n) ) ||
                ( (type[0]=='e')&& (set[i].anz_e) ) || 
                ( (type[0]=='f')&& (set[i].anz_f) ) ||  
                ( (type[0]=='p')&& (set[i].anz_p) ) ||  
                ( (type[0]=='l')&& (set[i].anz_l) ) ||  
                ( (type[0]=='s')&& (set[i].anz_s) ) ||  
                ( (type[0]=='b')&& (set[i].anz_b) ) ||  
    	    ( (type[0]=='L')&& (set[i].anz_nurl) ) ||
    	    ( (type[0]=='S')&& (set[i].anz_nurs) ) ))  break;
          }
          if(i<anz->sets) setNr=i;
        }
        sprintf(buffer,"minus");
        commandoInterpreter( buffer, keystroke2, pos, 0, 0, 0, 0 );
        sprintf(keystroke2, "%s %s %s %s", prognam, type, set[setNr].name, col);
        printf("%s %s %s(%d) %s\n", prognam, type, set[setNr].name, set[setNr].index, col);
        commandoInterpreter( prognam, keystroke2, pos, 0, 0, 0, 0 );

        /* history */
        if((key_history= (char **)realloc((char **)key_history, (nkey_history+2)*sizeof(char *))) == NULL )
        { printf("ERROR: malloc failed in Keyboard\n\n" ); return; }
        if((key_history[nkey_history]= (char *)malloc((MAX_LINE_LENGTH)*sizeof(char))) == NULL )
        { printf("ERROR: malloc failed in Keyboard\n\n" ); return; }
        strcpy(key_history[nkey_history],keystroke2);
        nkey_history++;
        key_pointer=nkey_history;
        return;
      }
    }

    /* default: display other loadcase */

    if(gkey==GLUT_KEY_PAGE_UP)
    {
      /* display the previous LC */
      for(lc=cur_lc-1; lc>=0; lc--)
      {
        if( compareStrings( lcase[lc].name,lcase[cur_lc].name)>0)
	{
          pre_lc=lc;
          break;
	}
      }
    }
    else if(gkey==GLUT_KEY_PAGE_DOWN)
    {
      /* display the next LC */
      for(lc=cur_lc+1; lc<anz->l; lc++)
      {
        if( compareStrings( lcase[lc].name,lcase[cur_lc].name)>0)
	{
          pre_lc=lc;
          break;
	}
      }
    }

    cur_lc=pre_lc;
    if (animFlag==1)
    {
      if(drawMode==1) sprintf(keystroke2, "ds %d a %d", pre_lc+1, cur_entity+1);
      else sprintf(keystroke2, "ds %d a", pre_lc+1);
    }
    else    sprintf(keystroke2, "ds %d e %d", pre_lc+1, cur_entity+1);
    printf("%s\n",keystroke2 );
    sprintf(prognam,"ds");

    if(addDispFlag==1)
    {
        addDispToCoordinates(node);
        addDispToCoordinates(node);
        printf("\n displacements of dataset:%d added. Scale them with 'scal d <value>'\n",pre_lc+1); 
    }

    commandoInterpreter( prognam, keystroke2, 2, 0, 0, 0, 0 );

    /* history */
    if((key_history= (char **)realloc((char **)key_history, (nkey_history+2)*sizeof(char *))) == NULL )
    { printf("ERROR: malloc failed in Keyboard\n\n" ); return; }
    if((key_history[nkey_history]= (char *)malloc((MAX_LINE_LENGTH)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in Keyboard\n\n" ); return; }
    strcpy(key_history[nkey_history],keystroke2);
    nkey_history++;
    key_pointer=nkey_history;
    return;
  }
}



void Keyboard( unsigned char gkey, int x, int y )
{
  int  j, pos, new_elems=0;
  int gtolFlag=0;
  static char  prognam[MAX_LINE_LENGTH];
  
  
  if ( parser( gkey, keystroke, &curshft) )
  {
    /* history */
    if((key_history= (char **)realloc((char **)key_history, (nkey_history+2)*sizeof(char *))) == NULL )
    { printf("ERROR: malloc failed in Keyboard\n\n" ); return; }
    if((key_history[nkey_history]= (char *)malloc((MAX_LINE_LENGTH)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in Keyboard\n\n" ); return; }
    strcpy(key_history[nkey_history],keystroke);
    nkey_history++;
    key_pointer=nkey_history;

    /* parser */
    pos = sword( keystroke, prognam);
    for(j=0;j<strlen(prognam); j++) prognam[j]=toupper(prognam[j]);

    if ( compare(prognam, "HELP", 4) == 4 ) help();
    else if ( compare(prognam, "QADD", 4) == 4 ) qadd(&keystroke[pos+1]);
    else if ( compare(prognam, "QALI", 4) == 4 ) qali();
    else if ( compare(prognam, "QBIA", 4) == 4 ) qbia();
    else if ( compare(prognam, "QBOD", 4) == 4 ) qbod(&keystroke[pos+1]);
    else if ( compare(prognam, "QCNT", 4) == 4 ) qcnt();
    else if ( compare(prognam, "QCUT", 4) == 4 ) qcut();
    else if ( compare(prognam, "QDEL", 4) == 4 ) qdel();
    else if ( compare(prognam, "QDIV", 4) == 4 ) qdiv();
    else if ( compare(prognam, "QDIS", 4) == 4 ) qdis();
    else if ( compare(prognam, "QMSH", 4) == 4 ) qmsh();
    else if ( compare(prognam, "QENQ", 4) == 4 ) qenq();
    else if ( compare(prognam, "QFIL", 4) == 4 ) qfil(&keystroke[pos+1]);
    else if ( compare(prognam, "QFLP", 4) == 4 ) qflp();
    else if ( compare(prognam, "QINT", 4) == 4 ) qint();
    else if ( compare(prognam, "QLIN", 4) == 4 ) qlin(&keystroke[pos+1]);
    else if ( compare(prognam, "QPNT", 4) == 4 ) qpnt(&keystroke[pos+1]);
    else if ( compare(prognam, "QNOD", 4) == 4 ) qnod();
    else if ( compare(prognam, "QNOR", 4) == 4 ) qnor();
    else if ( compare(prognam, "QREM", 4) == 4 ) qrem(&keystroke[pos+1]);
    else if ( compare(prognam, "QSUR", 4) == 4 ) qsur(&keystroke[pos+1]);
    else if ( compare(prognam, "QSEQ", 4) == 4 ) qseq(&keystroke[pos+1]);
    else if ( compare(prognam, "QPLN", 4) == 4 ) qshp(&keystroke[pos+1]);
    else if ( compare(prognam, "QSHP", 4) == 4 ) qshp(&keystroke[pos+1]);
    else if ( compare(prognam, "QSPL", 4) == 4 ) qspl();
    else if ( compare(prognam, "QTXT", 4) == 4 ) qtxt();
    else
    {
      commandoInterpreter( prognam, keystroke, pos, 0, 0, 0, &gtolFlag );
      if ( compare(prognam, "ELEM", 4) == 4 ) new_elems=1;
    }
    for (j=0; j<MAX_LINE_LENGTH; j++) prognam[j]=keystroke[j]='\0';
    curshft=0;

    //NEWELEM:
    if(new_elems)
    {
      new_elems=0;

      /* das neue netz muss noch zur beleuchteten ansicht aufbereitet werden  */
      adjustDrawNodes(1);
      getElemNormalen( e_enqire, node, anz->e );
      makeSurfaces();
      realloc_colNr();
      updateDispLists();       
    }
  }
}



void moveModel()
{
  glOrtho( -ds*aspectRatio_w1, ds*aspectRatio_w1, -ds, ds, -Z_DEPTH, Z_DEPTH ); /* nach glLoadIdentity () !! */
  
  v[0]= centerPnt[0] ;            /* nodes sind scaliert, sonst scalieren mit scalNodes() */
  v[1]= centerPnt[1] ;
  v[2]= centerPnt[2] ;
  v[3]=1.;
  m_sub( &dR[0][0], &R[0][0], &Rmem[0][0] );
  v_matmult( v, &dR[0][0] );
  glTranslated ( -v[0]-vmem[0], -v[1]-vmem[1], -v[2]-vmem[2] ); /* verschiebung durch verdrehung im Vorfeld */
                                            /* rueckgaengig machen */
  glTranslated ( dtx*ds, dty*ds, dtz );
  glMultMatrixd( &R[0][0] );
}



void DrawGraficLoad( void )
{
  //static int flipflop;
  double xc, yc, dxscal;
#if TEST
  printf(" in DrawGraficLoad\n");
#endif 

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] );
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  moveModel();
#if TEX_MODE
  /* enable all colors for the TEX_MODE */
  glColor3d( 1,1,1);
  // glEnable(GL_TEXTURE_1D);
#endif
  if (lcase[cur_lc].irtype == 3)
  {
    glCallList( list_elem_elstress );
  }
  else
  {
    if (surfFlag)   glCallList( list_surf_load );
    else            glCallList( list_elem_load );
  }
#if TEX_MODE
  // glDisable(GL_TEXTURE_1D);
#endif

  if (modelEdgeFlag)     glCallList( list_model_edges );
  if (elemEdgeFlag)
  {
    if (surfFlag) glCallList( list_surf_edges );
    else          glCallList( list_elem_edges );
  }

  if (bgpicture)
  {
    glLoadIdentity ();
    glRasterPos3f(-1., -1., 1.);
    glPixelZoom(bgpicture->zoom[0],bgpicture->zoom[1]);
    glDrawPixels( bgpicture->width, bgpicture->height, bgpicture->format, bgpicture->type, bgpicture->pixels);
  }
  if (pickFlag)
  {
    /* draw currend picking-Area  */
    xc= dx_cur/(double)width_w1;
    yc= dy_cur/(double)height_w1;
    //printf("dxy_cur:%f %f xyc:%f %f p:%f %f\n", dx_cur,dy_cur, xc,yc, dx-xc,dy+yc);
    glLoadIdentity ();
#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    dxscal=dx*height_w1/width_w1;
    glBegin ( GL_LINE_LOOP );
    glVertex3d ( dxscal-xc, dy+yc, -1. );
    glVertex3d ( dxscal+xc, dy+yc, -1. );
    glVertex3d ( dxscal+xc, dy-yc, -1. );
    glVertex3d ( dxscal-xc, dy-yc, -1. );
    glEnd();
  }

  glutSwapBuffers();

  if((movieFlag>0)&&(!stopFlag))
  {
    glutPostRedisplay();
    //if(flipflop) createHardcopy(3); flipflop=!flipflop;
    createHardcopy(3);
  }
  if(hcpyFlag)
  {
    if ( activWindow==w1 )  glutPostRedisplay();
    // if(flipflop) { createHardcopy(hcpyFlag); hcpyFlag=0; } flipflop=!flipflop;
    createHardcopy(hcpyFlag); hcpyFlag=0; 
  }
}



void DrawGraficLight( void )
{
  //static int flipflop;
  double xc, yc, dxscal;
#if TEST
  printf(" in DrawGraficLight\n");
#endif 

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] );
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  moveModel();
  if (surfFlag)    glCallList( list_surf_light );
  else             glCallList( list_elem_light );

  if (modelEdgeFlag) glCallList( list_model_edges );
  if (elemEdgeFlag)
  {
     if (surfFlag) glCallList( list_surf_edges );
     else          glCallList( list_elem_edges );
  }

  if (bgpicture)
  {
    glLoadIdentity ();
    glRasterPos3f(-1., -1., 1.);
    glPixelZoom(bgpicture->zoom[0],bgpicture->zoom[1]);
    glDrawPixels( bgpicture->width, bgpicture->height, bgpicture->format, bgpicture->type, bgpicture->pixels);
  }
  if (pickFlag)
  {
    /* draw currend picking-Area  */
    xc= dx_cur/(double)width_w1;
    yc= dy_cur/(double)height_w1;
    glLoadIdentity ();
#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    dxscal=dx*height_w1/width_w1;
    glBegin ( GL_LINE_LOOP );
      glVertex3d ( dxscal-xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy-yc, -1. );
      glVertex3d ( dxscal-xc, dy-yc, -1. );
    glEnd();
  }

  glutSwapBuffers();

  if((movieFlag>0)&&(!stopFlag))
  {
    glutPostRedisplay();
    //if(flipflop) createHardcopy(3); flipflop=!flipflop;
    createHardcopy(3);
  }
  if(hcpyFlag)
  {
    if ( activWindow==w1 )  glutPostRedisplay();
    //if(flipflop) { createHardcopy(hcpyFlag); hcpyFlag=0; } flipflop=!flipflop;
    createHardcopy(hcpyFlag); hcpyFlag=0;
  }
}



void DrawGraficAnimate( void )
{
  static char buffer[MAX_LINE_LENGTH];
  static int t0, t1;
  //static int flipflop;
  static double freqb, freqb_soll;
  double xc, yc, dxscal;
#if TEST
  printf(" in DrawGraficAnimate\n");
#endif 

 movieLoop:;

#if TEST
  frameNr++;
  /* Zeit stoppen  */
  if (frameNr==1) stopClock( frameNr );
  if (frameNr==100) {stopClock( frameNr ); frameNr=0;}
#endif

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] );
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  moveModel();
  glCallList( list_animate[animList] );
  if (modelEdgeFlag_Static)     glCallList( list_model_edges );
  if (modelEdgeFlag)     glCallList( list_animate_model_edges[animList] );
  if (elemEdgeFlag_Static)
  {
    if (surfFlag) glCallList( list_surf_edges );
    else          glCallList( list_elem_edges );
  }
  if (elemEdgeFlag)
  {
    if (surfFlag) glCallList( list_animate_surf_edges[animList] );
    else          glCallList( list_animate_elem_edges[animList] );
  }
  
  glLoadIdentity ();
  sprintf (buffer,"%4d%%Amplitude     ", anim_alfa[animList]);
#if INX_MODE
  glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
  glColor3dv( foregrndcol_rgb );
#endif
  text( -0.96, 0.96,-.99,buffer, glut_font[legend_font] );

  if (bgpicture)
  {
    glRasterPos3f(-1., -1., 1.);
    glPixelZoom(bgpicture->zoom[0],bgpicture->zoom[1]);
    glDrawPixels( bgpicture->width, bgpicture->height, bgpicture->format, bgpicture->type, bgpicture->pixels);
  }
  if (pickFlag)
  {
    /* draw currend picking-Area  */
    xc= dx_cur/(double)width_w1;
    yc= dy_cur/(double)height_w1;
#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    dxscal=dx*height_w1/width_w1;
    glBegin ( GL_LINE_LOOP );
      glVertex3d ( dxscal-xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy-yc, -1. );
      glVertex3d ( dxscal-xc, dy-yc, -1. );
    glEnd();
  }

  glutSwapBuffers();

  // keeps animation going when the mouse is in w1 
  if ( activWindow==w1 )  glutPostRedisplay();

  if (!stopFlag)
  {
    animList++;
    if ( animList>=anim_steps) animList=0;
  }

  /* because of a sgi-problem only one side of the graphics-buffer can be used for hcpy */
  /* therefore each frame is displayed twice if a movie is recorded */
  if((movieFlag>0)&&(!stopFlag))
  {
    printf("movieFrames:%d animList:%d anim_steps:%d gifNr:%d\n", movieFrames,animList,anim_steps,gifNr);
    if(movieFrames==-1) movieFrames=anim_steps+1;
    glutPostRedisplay();
    //if((!flipflop)&&(animList))  animList--;
    //else createHardcopy(3); flipflop=!flipflop;
    createHardcopy(3);
    if(animList<movieFrames) goto movieLoop;
  }
  if(hcpyFlag)
  {
    //if(flipflop) { createHardcopy(hcpyFlag); hcpyFlag=0; } flipflop=!flipflop;
    createHardcopy(hcpyFlag); hcpyFlag=0;
  }

  /* real-time wait */
  if(!movieFlag)
  {
    t0=t1;
    do {
      t1=clock();
      freqb = 1000 / ((t1-t0)+1) ;
      freqb_soll= anim_steps*1000 / time_per_period;
    } while ( freqb > freqb_soll );
  }
}



void DrawGraficSequence( void )
{
  static int j;
  static char buffer[MAX_LINE_LENGTH];
  static int t0, t1;
  //static int flipflop;
  static double freqb, freqb_soll;
  double xc, yc, dxscal;
  char key;
#if TEST
  printf(" in DrawGraficSequence\n");
#endif 

 movieLoop:;

#if TEST
  frameNr++;
  /* Zeit stoppen  */
  if (frameNr==1) stopClock( frameNr );
  if (frameNr==100) {stopClock( frameNr ); frameNr=0;}
#endif

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] );
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  moveModel();
  if (lightFlag); 
#if TEX_MODE
  else
  {
    /* enable all colors for the TEX_MODE */
    glColor3d( 1,1,1);
    glEnable(GL_TEXTURE_1D);
  }
#endif
  glCallList( list_animate[animList] );
  if (lightFlag); 
#if TEX_MODE
  else
  {
    glDisable(GL_TEXTURE_1D);
  }
#endif

  /* immediate draw the vectors. no display-list used because vector-length should be updated immediately */
  if ((vectorFlag)&&(v_dim>1))
  {
    if (modelEdgeFlag)     glCallList( list_model_edges );
    if (elemEdgeFlag)
    {
      if (surfFlag) glCallList( list_surf_edges );
      else          glCallList( list_elem_edges );
    }
    if(surfFlag) key='f';
    else         key='e';
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]==key)
      { 
        /* vectors keep their length:*/
        //if(key=='f') drawFaces_vector( dsSequence.ds[animList], v_dim, entity_v, v_factor*ds*0.1*v_scale, set[pset[j].nr].anz_f, set[pset[j].nr].face, node, face);  
        //if(key=='e') drawElements_vector( dsSequence.ds[animList], v_dim, entity_v, v_factor*ds*0.1*v_scale, set[pset[j].nr].anz_e, set[pset[j].nr].elem, node, e_enqire);
        /* scale vector length with zoom. */
        if(key=='f') drawFaces_vector( dsSequence.ds[animList], v_dim, entity_v, v_factor*0.025*v_scale, set[pset[j].nr].anz_f, set[pset[j].nr].face, node, face);  
        if(key=='e') drawElements_vector( dsSequence.ds[animList], v_dim, entity_v, v_factor*0.025*v_scale, set[pset[j].nr].anz_e, set[pset[j].nr].elem, node, e_enqire);
      }
    }
  }
  else
  {
    if (modelEdgeFlag_Static)     glCallList( list_model_edges );
    if (modelEdgeFlag)     glCallList( list_animate_model_edges[animList] );
    if (elemEdgeFlag_Static)
    {
      if (surfFlag) glCallList( list_surf_edges );
      else          glCallList( list_elem_edges );
    }
    if (elemEdgeFlag)
    {
      if (surfFlag) glCallList( list_animate_surf_edges[animList] );
      else          glCallList( list_animate_elem_edges[animList] );
    }
  }
  
  glLoadIdentity ();
  sprintf (buffer,"Frame:%d Time:%e %s%c", animList+1, lcase[lcase_animList].dat[animList][0], lcase[lcase_animList].dataset_text, '\0');
#if INX_MODE
  glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
  glColor3dv( foregrndcol_rgb );
#endif
  text( -0.96, 0.96, -0.99,buffer, glut_font[legend_font] );

  if (bgpicture)
  {
    glRasterPos3f(-1., -1., 1.);
    glPixelZoom(bgpicture->zoom[0],bgpicture->zoom[1]);
    glDrawPixels( bgpicture->width, bgpicture->height, bgpicture->format, bgpicture->type, bgpicture->pixels);
  }
  if (pickFlag)
  {
    /* draw currend picking-Area  */
    xc= dx_cur/(double)width_w1;
    yc= dy_cur/(double)height_w1;
    //glLoadIdentity ();
#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    dxscal=dx*height_w1/width_w1;
    glBegin ( GL_LINE_LOOP );
      glVertex3d ( dxscal-xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy-yc, -1. );
      glVertex3d ( dxscal-xc, dy-yc, -1. );
    glEnd();
  }

  glutSwapBuffers();

  if ( activWindow==w1 )  glutPostRedisplay();

  if (!stopFlag)
  {
    animList++;
    if ( animList>=dsSequence.nds) animList=0;
  }

  /* because of a sgi-problem only one side of the graphics-buffer can be used for hcpy */
  /* therefore each frame is displayed twice if a movie is recorded */
  if((movieFlag>0)&&(!stopFlag))
  {
    if(movieFrames==-1) movieFrames=dsSequence.nds;
    glutPostRedisplay();
    //if((!flipflop)&&(animList))  animList--;
    //else createHardcopy(3); flipflop=!flipflop;
    createHardcopy(3);
    if(animList<movieFrames) goto movieLoop;
  }
  if(hcpyFlag)
  {
    //if(flipflop) { createHardcopy(hcpyFlag); hcpyFlag=0; } flipflop=!flipflop;
    createHardcopy(hcpyFlag); hcpyFlag=0;
  }

  /* real-time wait */
  if(!movieFlag)
  {
    t0=t1;
    do {
    t1=clock();
    freqb = 1000 / ((t1-t0)+1) ;
    freqb_soll= anim_steps*1000 / time_per_period;
    } while ( freqb > freqb_soll );
  }
}



void DrawPickedItems()
{
  //static int flipflop;
  double xc, yc, dxscal;
#if TEST
  printf(" in DrawPickedItems\n");
#endif 

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] );
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  moveModel();
  drawSets(!PICK);
  if (modelEdgeFlag) glCallList( list_model_edges );

  if (bgpicture)
  {
    glLoadIdentity ();
    glRasterPos3f(-1., -1., 1.);
    glPixelZoom(bgpicture->zoom[0],bgpicture->zoom[1]);
    glDrawPixels( bgpicture->width, bgpicture->height, bgpicture->format, bgpicture->type, bgpicture->pixels);
  }
  if (pickFlag)
  {
    /* draw currend picking-Area  */
    xc= dx_cur/(double)width_w1;
    yc= dy_cur/(double)height_w1;
    glLoadIdentity ();
#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    dxscal=dx*height_w1/width_w1;
    glBegin ( GL_LINE_LOOP );
      glVertex3d ( dxscal-xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy+yc, -1. );
      glVertex3d ( dxscal+xc, dy-yc, -1. );
      glVertex3d ( dxscal-xc, dy-yc, -1. );
    glEnd();
  }

  glutSwapBuffers();

  if((movieFlag>0)&&(!stopFlag))
  {
    glutPostRedisplay();
    //if(flipflop) createHardcopy(3); flipflop=!flipflop;
    createHardcopy(3);
  }
  if(hcpyFlag)
  {
    if ( activWindow==w1 )  glutPostRedisplay();
    //if(flipflop) { createHardcopy(hcpyFlag); hcpyFlag=0; } flipflop=!flipflop;
    createHardcopy(hcpyFlag); hcpyFlag=0;
  }
}


void drawSets(int mode)
{
  int j;

#if TEST
  printf(" in drawSets\n");
#endif 

  for (j=0; j<anzGeo->psets; j++ )
  {
    if (pset[j].type[0]=='n')
    {
      drawNodes_plot( set[pset[j].nr].anz_n, set[pset[j].nr].node, node, pset[j].col, pset[j].type[1] );
      if ((vectorFlag)&&(v_dim>1))
      {
        /* vectors keep their length:*/
        // drawNodes_vector( dsSequence.ds[0], v_dim, entity_v, v_factor*ds*0.1*v_scale, set[pset[j].nr].anz_n, set[pset[j].nr].node, node);  
        /* scale vector length with zoom. */
        drawNodes_vector( dsSequence.ds[0], v_dim, entity_v, v_factor*0.025*v_scale, set[pset[j].nr].anz_n, set[pset[j].nr].node, node);  
      }
    }
    if (pset[j].type[0]=='e')
    {
      if(mode) drawElemNodes_plot( set[pset[j].nr].anz_e, set[pset[j].nr].elem, node, e_enqire, 2, 0 );
      if(elemEdgeFlag) drawElem_edge( set[pset[j].nr].anz_e, set[pset[j].nr].elem, node, e_enqire, basCol[0], pset[j].type[1] );
      drawElements_plot( set[pset[j].nr].anz_e, set[pset[j].nr].elem, node, colNr, e_enqire, pset[j].col, pset[j].type[1], mode );
    }
    if (pset[j].type[0]=='f')
    {
      if(mode) drawFaceNodes_plot( set[pset[j].nr].anz_f, set[pset[j].nr].face, node, face, 2, 0 );
      if(elemEdgeFlag) drawFaces_edge( set[pset[j].nr].anz_f, set[pset[j].nr].face, node, face, basCol[0], pset[j].type[1] );
      drawFaces_plot( set[pset[j].nr].anz_f, set[pset[j].nr].face, node, colNr, face, pset[j].col, pset[j].type[1], mode );
    }
    if (pset[j].type[0]=='p')
    {
      drawPoints_plot( set[pset[j].nr].anz_p, set[pset[j].nr].pnt, point, pset[j].col, pset[j].type[1] );
    }
    if (pset[j].type[0]=='l')
    {
      drawLines_plot( set[pset[j].nr].anz_l, set[pset[j].nr].line, line, point, pset[j].col, pset[j].type[1] );
    }
    if (pset[j].type[0]=='s')
    {
      if(pset[j].type[1]=='h') drawShapes_plot( set[pset[j].nr].anz_sh, set[pset[j].nr].shp, shape, point, pset[j].col, pset[j].type[1]);
#if INX_MODE
      else drawSurfs_plot( set[pset[j].nr].anz_s, set[pset[j].nr].surf, surf, lcmb, line, point, pset[j].col, pset[j].type[1] );
#endif
#if TEX_MODE
      else
      {
        if(pset[j].type[1]=='i') setLightAndMaterial_rgb(pset[j].col);
        drawSurfs_plot( set[pset[j].nr].anz_s, set[pset[j].nr].surf, surf, lcmb, line, point, pset[j].col, pset[j].type[1] );
      }
#endif
    }
    if (pset[j].type[0]=='b')
    {
      drawBodys_plot( set[pset[j].nr].anz_b, set[pset[j].nr].body, body, surf, lcmb, line, point, pset[j].col, pset[j].type[1] );
    }
    if (pset[j].type[0]=='L')
    {
      drawNurl_plot( set[pset[j].nr].anz_nurl, set[pset[j].nr].nurl, pset[j].col, pset[j].type[1], mode );
    }
    if (pset[j].type[0]=='S')
    {
#if TEX_MODE
      setLightAndMaterial_rgb(pset[j].col);
#endif
      drawNurs_plot( set[pset[j].nr].anz_nurs, set[pset[j].nr].nurs, pset[j].col, pset[j].type[1], mode );
    }
  }
}



void DrawAxes()
{
  static char buffer[MAX_LINE_LENGTH];
#if TEST
  printf(" in DrawGraficAxes\n");
#endif 

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] );
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  glOrtho( -1., 1., -1., 1., -1., 1. ); /* nach glLoadIdentity () !! */

    v[0]= centerPnt[0] ;            /* nodes sind scaliert, sonst scalieren mit scalNodes() */
    v[1]= centerPnt[1] ;
    v[2]= centerPnt[2] ;
    v[3]=1.;
    m_sub( &dR[0][0], &R[0][0], &Rmem[0][0] );
    v_matmult( v, &dR[0][0] );
    glMultMatrixd( &R[0][0] );

#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    glBegin ( GL_LINE_STRIP );
     glVertex3d(0.,.5,0.);
     glVertex3d(0.,0.,0.);
     glVertex3d(.5,0.,0.);
    glEnd();
    glBegin ( GL_LINES );
     glVertex3d(0.,0.,0.);
     glVertex3d(0.,0.,.5);
    glEnd();
  
    text( .5, 0., 0., "X ", glut_font[legend_font] );
    text( 0., .5, 0., "Y ", glut_font[legend_font] );
    text( 0., 0., .5, "Z ", glut_font[legend_font] );

  if(drawMode!=4)
  {
    glLoadIdentity ();
    if(surfFlag) sprintf (buffer,"s");
    else         sprintf (buffer,"v");
#if INX_MODE
    glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
    glColor3dv( foregrndcol_rgb );
#endif
    text( -1, -1, 0.,buffer, glut_font[legend_font] );
  }    
  glutSwapBuffers();
}



void idleFunction(void)
{
  int i,k;
#if TEST
  printf(" in Idle\n");
#endif 

  glutSetWindow( w0);
  glutDisplayFunc ( DrawMenuSet );
  DrawMenuSet();
  glutSetWindow( w1);
  glutDisplayFunc ( DrawPickedItems );
  DrawPickedItems();
  glutSetWindow( w2);
  DrawAxes();

  if(iniActionsFlag)
  {
    /* read the geometry of the model */
    if(iniActionsFlag==1) { readfbd( datin, 0);  if(!animFlag) updateDispLists(); }
    if(iniActionsFlag==2)
    {
      i=readstep( datin, step_mode);
      sprintf(picture_text,"displayed shape: %s", set[i].name);
    }
#if INX_MODE
    defineColIndexes_light();
#endif
    iniActionsFlag=0;

    /* if a cad file was read do some preparations */
    if(automode==1)
    {
      /* merge only points which are referenced by lines */
      printf("merge only points which are referenced by lines\n");
      k=pre_seta(specialset->uori, "i", 0 );
      for (i=0; i<anzGeo->l; i++) seta(k,"l",i);
      completeSet( specialset->uori, "e") ;
      sprintf( buffer,"p %s %lf nolock", specialset->uori, gtol*GTOL_EDGES);
      pre_merge( buffer);

      /* delete zero length lines */
      printf("delete zero length lines\n");
      sprintf( buffer,"l0 %s", specialset->uori);
      pre_del( buffer);

      /* in case NURLs are read then calculate a suitable div for the related spline */
      printf("calcLineDiv\n");
      for (i=0; i<set[k].anz_l; i++)
      {
        calcLineDiv(line, set[k].line[i], GTOL_COS_A, gtol*GTOL_NODE_DIST/scale->w, gtol*GTOL_NODE_DIST/scale->w*ELEM_LENGTH_RATIO);
      } 
      delSet(specialset->uori );

      glutSetWindow( w1 ); /* has to be called before repSurf() to use the correct resolution */
      /* glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside); */
      /* glCullFace ( GL_FRONT ); */
      /* set "all" to avoid substitute surfs */
      printf("orientSet\n");
      orientSet( "all" );
      for (i=0; i<set[setall].anz_l; i++) repLine(set[setall].line[i]);
      //for (i=0; i<set[setall].anz_s; i++) repSurf(set[setall].surf[i], 1);
      //pre_body("B1 all");
      plot("lp all\n"); 
      //plus("ba all\n");
      printf("\n Create separate bodies (body ! setname (ie. all))\n");
      printf(" Choose the element-type with 'elty' (ie. 'elty all te10 (or tr6u for surfs)') and mesh with 'mesh all'.\n\n");
      updateDispLists();
    }
  }

  if(anzGeo->psets==0)
  {
    plot("p all    \n");
    plus("l all    \n"); 
    plus("s all    \n"); 
    plus("b all    \n");
  }

  /* deactivate the idleFunction */
  glutIdleFunc (NULL);
}



void iniDrawMenu()
{
  char buffer[MAX_LINE_LENGTH];
  int i, maxchars,pixpchar=9;
  double x;
#if TEST
  printf(" in iniDrawMenu\n");
#endif 

#if INX_MODE
  glClearIndex ( basCol[backgrndcol] );
#endif
#if TEX_MODE
  glClearColor ( backgrndcol_rgb[0], backgrndcol_rgb[1], backgrndcol_rgb[2], backgrndcol_rgb[3] ); 
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  glOrtho( -1., 1., -1., 1., -1., 1. ); /* nach glLoadIdentity () !! */

#if INX_MODE
  glIndexi  ( basCol[foregrndcol] );
#endif
#if TEX_MODE
  glColor3dv( foregrndcol_rgb );
#endif
  if (frameFlag)
  {
    glBegin ( GL_LINE_LOOP );
     glVertex3d(-1+(width_menu*19/20-1)*2./width_w0, 1-(height_menu/10-1)*2./height_w0, 0. );
     glVertex3d(-1+(width_menu*19/20-1)*2./width_w0, 1-(height_menu/10+height_w1+1)*2./height_w0, 0. );
     glVertex3d(-1+(width_menu*19/20+width_w1+1)*2./width_w0,1-(height_menu/10+height_w1+1)*2./height_w0,0.);
     glVertex3d(-1+(width_menu*19/20+width_w1+1)*2./width_w0, 1-(height_menu/10-1)*2./height_w0, 0. );
    glEnd();
  }
  if (filenamFlag)
  {
    maxchars=(width_w0 /pixpchar);
    i=strlen(datin)-maxchars;
    if (i>0) strcpy(buffer, &datin[i]);
    else strcpy(buffer, datin);
    x=  -(double)(strlen( buffer )*pixpchar) / (double)(width_w0);
    text( x, -0.91, 0., buffer, glut_font[legend_font] );
  }
  if (textFlag)
  {
    maxchars=(width_w0 /pixpchar);
    i=strlen(picture_text)-maxchars;
    if (i>0) strcpy(buffer, &picture_text[i]);
    else strcpy(buffer, picture_text);
    x=  -(double)(strlen( buffer )*pixpchar) / (double)(width_w0);
    text( x, -0.97, 0., buffer, glut_font[legend_font] );
  }
}



void DrawMenuLoad( void )
{
  char buffer[MAX_LINE_LENGTH];
#if INX_MODE
  defineColIndexes_load();
#endif
#if TEX_MODE
  defineColTextur_load();
#endif

  iniDrawMenu();
  if (scalaFlag)
  {
    sprintf (buffer,"%d/%d:%s", lcase[cur_lc].step_number,cur_lc+1,lcase[cur_lc].name);
    text( -0.99, 0.95, 0.,buffer, glut_font[legend_font]);
    sprintf (buffer,"Time:%lf", lcase[cur_lc].value);
    text( -0.99, 0.9, 0.,buffer, glut_font[legend_font]);
    sprintf (buffer,"Entity:%s", lcase[cur_lc].compName[cur_entity]);
    text( -0.99, 0.85, 0., buffer, glut_font[legend_font] );
    if(addDispFlag)
    {
      sprintf (buffer,"+Dispf:%lf", anim_faktor);
      text( -0.99, 0.80, 0., buffer, glut_font[legend_font] );
    }
    if(scale->format=='f')
    {
      sprintf (buffer,"max: %-10f  ", lcase[cur_lc].max[cur_entity]);
      text( -0.99, 0.75, 0., buffer, glut_font[legend_font] );
      sprintf (buffer,"min: %-10f  ", lcase[cur_lc].min[cur_entity]);
      text( -0.99, 0.71, 0., buffer, glut_font[legend_font] );
    }
    else if(scale->format=='i')
    {
      sprintf (buffer,"max: %-10f  ", lcase[cur_lc].max[cur_entity]);
      text( -0.99, 0.75, 0., buffer, glut_font[legend_font] );
      sprintf (buffer,"min: %-10f  ", lcase[cur_lc].min[cur_entity]);
      text( -0.99, 0.71, 0., buffer, glut_font[legend_font] );
    }
    else
    {
      sprintf (buffer,"max: %5.2e  ", lcase[cur_lc].max[cur_entity]);
      text( -0.99, 0.75, 0., buffer, glut_font[legend_font] );
      sprintf (buffer,"min: %5.2e  ", lcase[cur_lc].min[cur_entity]);
      text( -0.99, 0.71, 0., buffer, glut_font[legend_font] );
    }
#if INX_MODE
    scala_indx( -0.93, -0.85, steps, scale->smin, scale->smax, offset, basCol[foregrndcol], glut_font[legend_font], scale->format);
#endif
#if TEX_MODE
    if ((animFlag==0)||(halfperiod)) scala_tex( -0.93, -0.85, steps, scale->smin, scale->smax, (double)steps/(double)TEX_PIXELS, foregrndcol_rgb, glut_font[legend_font], scale->format);
    else  scala_tex( -0.93, -0.85, steps, scale->smin-(scale->smax-scale->smin), scale->smax, (double)steps/(double)TEX_PIXELS, foregrndcol_rgb, glut_font[legend_font], scale->format);
#endif
  }
  glutSwapBuffers();
}



void DrawMenuSequence( void )
{
  char buffer[MAX_LINE_LENGTH];
#if INX_MODE
  defineColIndexes_load();
#endif
#if TEX_MODE
  defineColTextur_load();
#endif

  iniDrawMenu();
  if (scalaFlag)
  {
    sprintf (buffer,"%d/%d:%s", lcase[lcase_animList].step_number, lcase_animList+1,lcase[lcase_animList].name);
    text( -0.99, 0.95, 0.,buffer, glut_font[legend_font]);
    sprintf (buffer,"Entity:%s", lcase[lcase_animList].compName[0]);
    text( -0.99, 0.85, 0., buffer, glut_font[legend_font] );
    if(addDispFlag)
    {
      sprintf (buffer,"+Dispf:%lf", anim_faktor);
      text( -0.99, 0.80, 0., buffer, glut_font[legend_font] );
    }
    if(scale->format=='f')
    {
      sprintf (buffer,"max: %-10f  ", lcase[cur_lc].max[0]);
      text( -0.99, 0.75, 0., buffer, glut_font[legend_font] );
      sprintf (buffer,"min: %-10f  ", lcase[cur_lc].min[0]);
      text( -0.99, 0.71, 0., buffer, glut_font[legend_font] );
    }
    else if(scale->format=='i')
    {
      sprintf (buffer,"max: %-10f  ", lcase[cur_lc].max[0]);
      text( -0.99, 0.75, 0., buffer, glut_font[legend_font] );
      sprintf (buffer,"min: %-10f  ", lcase[cur_lc].min[0]);
      text( -0.99, 0.71, 0., buffer, glut_font[legend_font] );
    }
    else
    {
      sprintf (buffer,"max: %5.2e  ", lcase[cur_lc].max[0]);
      text( -0.99, 0.75, 0., buffer, glut_font[legend_font] );
      sprintf (buffer,"min: %5.2e  ", lcase[cur_lc].min[0]);
      text( -0.99, 0.71, 0., buffer, glut_font[legend_font] );
    }
#if INX_MODE
    scala_indx( -0.93, -0.85, steps, scale->smin, scale->smax, offset, basCol[foregrndcol], glut_font[legend_font], scale->format);
#endif
#if TEX_MODE
    scala_tex( -0.93, -0.85, steps, scale->smin, scale->smax, (double)steps/(double)TEX_PIXELS, foregrndcol_rgb, glut_font[legend_font], scale->format);
#endif
  }
  glutSwapBuffers();
}



void DrawMenuLight( void )
{
  iniDrawMenu();
    if(addDispFlag)
    {
      sprintf (buffer,"+Dispf:%lf", anim_faktor);
      text( -0.99, 0.80, 0., buffer, glut_font[legend_font] );
    }
  glutSwapBuffers();
}



void DrawMenuAnimate( void )
{
  char buffer[MAX_LINE_LENGTH];

  iniDrawMenu();
  if ((!sequenceFlag)&&(cur_lc > -1))
  {
    sprintf (buffer,"%d/%d:%s", lcase[cur_lc].step_number, cur_lc+1,lcase[cur_lc].name);
    text( -0.99, 0.95, 0.,buffer, glut_font[legend_font]);
    sprintf (buffer,"Time:%lf", lcase[cur_lc].value);
    text( -0.99, 0.9, 0.,buffer, glut_font[legend_font]);
  }
  text( -0.99, 0.85, 0., "Animated", glut_font[legend_font] );
  if (halfperiod)
  {
    text( -0.99, 0.8, 0., "Tune-value:", glut_font[legend_font] );
    sprintf (buffer,"%3.1f", anim_faktor);
    text( -0.99, 0.76, 0., buffer, glut_font[legend_font] );
  }
  glutSwapBuffers();
}



void DrawMenuSet( void )
{
#if INX_MODE
  defineColIndexes_set();
#endif
  iniDrawMenu();
  glutSwapBuffers();
}



void reshape( int width, int height )
{
  width_w0=width; 
  height_w0=height; 
  /* MAIN WINDOW */
  glutSetWindow( w0 );
    glViewport(0, 0, (GLint)width, (GLint)height ); 
    width_w1=width - width_menu; 
    height_w1=height - height_menu; 
    aspectRatio_w1=(double)width_w1/(double)height_w1;
  /* Drawing window */
  glutSetWindow( w1 ); 
    glutPositionWindow( width_menu*19/20, height_menu/10); 
    glutReshapeWindow( width_w1, height_w1); 
    glViewport( 0, 0, (GLint)width_w1, (GLint)height_w1); 
  /* axis window */
  glutSetWindow( w2 ); 
    glutPositionWindow( 0, height_w1*0.9); 
    glutReshapeWindow( height_w1/10, height_w1/10); 
    glViewport( 0, 0, (GLint)height_w1/10, (GLint)height_w1/10);
  glutPostRedisplay();
}



void initLightAndMaterial_rgb( void )
{
    static GLfloat lmodel_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

  /* lichtanteile rgba definieren  */
    static GLfloat ambient0[] = { AMB, AMB, AMB, 1.0 }; /* ungerichtet */
    static GLfloat diffuse0[] = { DIFF, DIFF, DIFF, 1.0 }; /* gerichtetes licht*/

  /* Materialeigenschaften definieren */
    static GLfloat mat_shininess[] = { 128. };             /* 0->128 Blankheit */
    static GLfloat mat_specular[] = { MAT_SPEC,MAT_SPEC,MAT_SPEC, 1.0 }; /* ungerichtete reflektion*/
    static GLfloat mat_diffuse[] = { MAT_DIFF,MAT_DIFF,MAT_DIFF, 1.0 };  /* gerichtete reflektion*/

  /* Position der Lichtquellen xyzw, mit w=0.0 entfernung unendlich def.*/
    static GLfloat position0[] = { .0, .0, -1., 0.0 };

    /*    glMatrixMode(GL_PROJECTION); fuert hier zu bildfehlern (Z-Buffer) */

    /* Definieren und Positionieren der Lampen */
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);          /* allseitiges Licht */
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);          /* gerichtetes Licht, ungerichtete reflexion */
    glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse0);         /* gerichtetes Licht, gerichtete reflexion */
    glLightfv(GL_LIGHT0, GL_POSITION, position0);

    /* Reflexionseigenschaften des Materials aufbringen */
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    askGLError("initLightAndMaterial_rgb,GL_SHININESS");
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_specular);

    /* Beschreibung des Beleuchtungsmodells */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient); /* globales licht ohne quelle */
    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE); /* GL_FALSE: infinite distance */

    glEnable(GL_LIGHT0);
    glDisable (GL_COLOR_MATERIAL);      /* improves performance (~2%) */
}



void initLightAndMaterial_index( void )
{
  /* lichtanteile rgba definieren  */
  static GLfloat diffuse0[] = { DIFF_I,DIFF_I,DIFF_I, 1.0 };   /* gerichtetes licht, ungerichtete reflexion */
  static GLint shininess = { 128 };              /* 0->128 Blankheit (geht nicht!)*/

  /* Position der Lichtquellen xyzw, mit w=0.0 entfernung unendlich def.*/
  static GLfloat position0[] = { .0, .0, -1., 0.0 };

  glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, shininess );
  askGLError("initLightAndMaterial_index,GL_SHININESS");

  /* Definieren und Positionieren der Lampen */
  /*    glLightfv(GL_LIGHT0, GL_AMBIENT, ----- ); does not work in Index-mode */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
  askGLError("initLightAndMaterial_index,GL_DIFFUSE");
  /*    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0); has no effect */
  glLightfv(GL_LIGHT0, GL_POSITION, position0);
  askGLError("initLightAndMaterial_index,GL_POSITION");

  /* Beschreibung des Beleuchtungsmodells */
  glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
  askGLError("initLightAndMaterial_index,GL_LIGHT_MODEL_TWO_SIDE");
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);       /* GL_FALSE: infinite distance */
  askGLError("initLightAndMaterial_index,GL_LIGHT_MODEL_LOCAL_VIEWER");

  glEnable(GL_LIGHT0);
}


int main ( int argc, char **argv )
{
  int i,j;
  char buffer[MAX_LINE_LENGTH];
  static char ccxfile[MAX_LINE_LENGTH];


  lcase=NULL;
  node=NULL;
  e_enqire=NULL;
  anz->orign=0;
  anz->n=0;
  anz->e=0;
  anz->f=0;
  anz->g=0;
  anz->t=0;
  anz->l=0;
  anz->olc=0;
  anz->orignmax=0;
  anz->nmax=0;
  anz->nmin=MAX_INTEGER;
  anz->emax=0;
  anz->emin=MAX_INTEGER;
  anz->sets=0;
  anz->mats=0;
  anz->amps=0;
  anz->noffs=0;
  anz->eoffs=0;

  anzGeo->p=0;
  anzGeo->l=0;
  anzGeo->c=0;
  anzGeo->s=0;
  anzGeo->b=0;
  anzGeo->sh=0;
  anzGeo->nurl=0;
  anzGeo->nurs=0;
  anzGeo->psets=0;

  openSets->nr=0;
  openSets->set=NULL;

  copiedNodeSets->sets=0;
  copiedNodeSets->type=NULL;
  copiedNodeSets->anz_n=NULL;
  copiedNodeSets->axis=NULL;
  copiedNodeSets->mnod=NULL;
  copiedNodeSets->snod=NULL;
  copiedNodeSets->fi=NULL;

  dsSequence.nds=0;
  dsSequence.ds=NULL;

  strcpy(specialset->njby,NJBY);
  strcpy(specialset->copy,COPY);
  strcpy(specialset->mesh,MESH);
  strcpy(specialset->nomesh,NOMESH);
  strcpy(specialset->zap,ZAP);
  strcpy(specialset->impc,IMPC);
  strcpy(specialset->mpc,DMPC);
  strcpy(specialset->nompc,NOMPC);
  strcpy(specialset->noel,NOEL);
  strcpy(specialset->ori,ORI);
  strcpy(specialset->dep,DEP);
  strcpy(specialset->ind,IND);
  strcpy(specialset->tmp,TMP);
  strcpy(specialset->highl,HIGHLIGHT);
  strcpy(specialset->uori,UORI);
  strcpy(specialset->bnur,BNUR);
  strcpy(specialset->bsur,BSUR);
  strcpy(specialset->noprj,NOPRJ);
  strcpy(specialset->cf,CF);
  strcpy(specialset->plot2d,PLOT2D);

  resetScaleValues( scale);
  scale->format='e';
  scale->lock='u';

  sumAsci->max_suma=0;
  sumAsci->anza=NULL;
  sumAsci->aindx=NULL;
  sumAsci->max_sump=0;
  sumAsci->anzp=NULL;
  sumAsci->pindx=NULL;
  sumAsci->max_suml=0;
  sumAsci->anzl=NULL;
  sumAsci->lindx=NULL;
  sumAsci->max_sumc=0;
  sumAsci->anzc=NULL;
  sumAsci->cindx=NULL;
  sumAsci->max_sums=0;
  sumAsci->anzs=NULL;
  sumAsci->sindx=NULL;
  sumAsci->max_sumb=0;
  sumAsci->anzb=NULL;
  sumAsci->bindx=NULL;
  sumAsci->max_sumS=0;
  sumAsci->anzS=NULL;
  sumAsci->Sindx=NULL;
  sumAsci->max_sumse=0;
  sumAsci->anzse=NULL;
  sumAsci->seindx=NULL;
  sumAsci->max_sumsh=0;
  sumAsci->anzsh=NULL;
  sumAsci->shindx=NULL;
  sumAsci->max_sumamp=0;
  sumAsci->anzamp=NULL;
  sumAsci->ampindx=NULL;


  setall=seto( "all" );

  /* get the current system type  */
  uname (cursys);
  printf ("on a %s machine, ", cursys->sysname);
  printf ("nodename %s, ", cursys->nodename);
  printf ("release %s, ", cursys->release);
  printf ("version %s, ", cursys->version);
  printf ("machine %s \n", cursys->machine);

  if (getenv("CGXDOCS")!=NULL) {
  	sprintf(helpfile[0],"%s",getenv("CGXDOCS"));
  }
  if (getenv("CCXDOCS")!=NULL) {
  	sprintf(helpfile[1],"%s",getenv("CCXDOCS"));
  }
  if (getenv("BROWSER")!=NULL) {
  	sprintf(browser,"%s",getenv("BROWSER"));
  }
  if (getenv("CGXTERMVIEWER")!=NULL) {
  	sprintf(psviewer,"%s",getenv("CGXTERMVIEWER"));
  }

  if (argc < 2)
  {
    generalinfo();
    exit (0);
  }
  else
  {
    inpformat=0;
    i=0;
    do{ i++;} while((i<argc)&&(argv[i][0]=='-'));
    printf("parameters:%d arguments:%d\n", argc, i);
    for (j=1; j<i; j++)
    {
      if( compare( argv[j], "-a", 2 ) == 2 )
      { 
        inpformat='b';
        automode=1;
        if(argc==i) strcpy( datin, "dummy.fbd"); 
        else strcpy( datin, argv[i]);
        if ( (lcase = (Datasets *)malloc( (anz->l+1) * sizeof(Datasets))) == NULL )
          printf("\n\n ERROR: malloc failed lcase\n\n") ;
        if ( (node = (Nodes *)malloc( (anz->n+1) * sizeof(Nodes))) == NULL )
          printf("\n\n ERROR: malloc failed node\n\n") ;
        if ( (e_enqire = (Elements *)malloc( (anz->e+1) * sizeof(Elements))) == NULL )
          printf("\n\n ERROR: malloc failed elem\n\n") ;
      }
      else if( compare( argv[j], "-bg", 3 ) == 3 )
      { 
        inpformat=0;
        if(argc==i) strcpy( datin, "dummy.fbd"); 
        else strcpy( datin, argv[i]);
        if ( (lcase = (Datasets *)malloc( (anz->l+1) * sizeof(Datasets))) == NULL )
          printf("\n\n ERROR: malloc failed lcase\n\n") ;
        if ( (node = (Nodes *)malloc( (anz->n+1) * sizeof(Nodes))) == NULL )
          printf("\n\n ERROR: malloc failed node\n\n") ;
        if ( (e_enqire = (Elements *)malloc( (anz->e+1) * sizeof(Elements))) == NULL )
          printf("\n\n ERROR: malloc failed elem\n\n") ;
        readfbd( datin, 0);
        exit(0);
      }
      else if( compare( argv[j], "-b", 2 ) == 2 )
      { 
        inpformat='b';
        if(argc==i) strcpy( datin, "dummy.fbd"); 
        else strcpy( datin, argv[i]);
        if ( (lcase = (Datasets *)malloc( (anz->l+1) * sizeof(Datasets))) == NULL )
          printf("\n\n ERROR: malloc failed lcase\n\n") ;
        if ( (node = (Nodes *)malloc( (anz->n+1) * sizeof(Nodes))) == NULL )
          printf("\n\n ERROR: malloc failed node\n\n") ;
        if ( (e_enqire = (Elements *)malloc( (anz->e+1) * sizeof(Elements))) == NULL )
          printf("\n\n ERROR: malloc failed elem\n\n") ;
      }
      else if( compare( argv[j], "-c", 2 ) == 2 ) 
      { 
        inpformat='c';
        if(argc==i) { printf("ERROR: no file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 
      else if( compare( argv[j], "-duns2d", 7 ) == 7 ) 
      { 
        inpformat='d';
        if(argc==i) { printf("ERROR: no duns-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 
      else if( compare( argv[j], "-duns3d", 7 ) == 7 ) 
      { 
        inpformat='e';
        if(argc==i) { printf("ERROR: no duns-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 
      else if( compare( argv[j], "-foam", 5 ) == 5 ) 
      { 
        inpformat='f';
        if(argc==i) { printf("ERROR: no foam-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 
      else if( compare( argv[j], "-isaac2d", 8 ) == 8 ) 
      { 
        inpformat='i';
        strcpy( datin, argv[argc-1]);
      } 
      else if( compare( argv[j], "-isaac3d", 8 ) == 8 ) 
      { 
        inpformat='j';
        strcpy( datin, argv[argc-1]);
      } 
      else if( compare( argv[j], "-f06", 4 ) == 4 ) 
      { 
        inpformat='m';
        if(argc==i) { printf("ERROR: no ng-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 
      else if( compare( argv[j], "-ng", 3 ) == 3 ) 
      { 
        inpformat='n';
        if(argc==i) { printf("ERROR: no ng-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 
      else if( compare( argv[j], "-oldbias", 7 ) == 7 )  { OLD_BIAS_DEF=1; }
      else if( compare( argv[j], "-read", 5 ) == 5 ) 
      { 
        read_mode=1;
      } 
      else if( compare( argv[j], "-step", 5 ) == 5 ) 
      { 
        inpformat='x';
        if(argc==i) { printf("ERROR: no step-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]);

        if( compare( argv[j], "-stepsplit", 10 ) == 10 ) step_mode=1;

        if ( (lcase = (Datasets *)malloc( (anz->l+1) * sizeof(Datasets))) == NULL )
          printf("\n\n ERROR: malloc failed lcase\n\n") ;
        if ( (node = (Nodes *)malloc( (anz->n+1) * sizeof(Nodes))) == NULL )
          printf("\n\n ERROR: malloc failed node\n\n") ;
        if ( (e_enqire = (Elements *)malloc( (anz->e+1) * sizeof(Elements))) == NULL )
          printf("\n\n ERROR: malloc failed elem\n\n") ;
      } 
      else if( compare( argv[j], "-stl", 4 ) == 4 ) 
      { 
        inpformat='s';
        if(argc==i) { printf("ERROR: no stl-file specified\n"); exit(-1); } 
        else strcpy( datin, argv[i]); 
      } 

      /* special purpose, undocumented features */
      else if( compare( argv[j], "-pref", 5 ) == 5 )  { pref=atof(&argv[j][5]); }
      else if( compare( argv[j], "-R", 2 ) == 2 )     { R_GAS=atof(&argv[j][2]); }
      else if( compare( argv[j], "-tref", 5 ) == 5 )  { tref=atof(&argv[j][5]); }

      if( compare( argv[j], "--v", 3 ) == 3 ) { printf("Version %s\n", VERSION);  exit (0); } 
    }
    if (inpformat==0)
    {
      inpformat='v';
      if(argc==i) { printf("ERROR: no file specified\n"); exit(0); }
      else if(argc==i+2) strcpy( ccxfile, argv[i+1]); 
      strcpy( datin, argv[i]); 
    }
  }


  /* Trackballfunktion inizialisieren  */
  trackball( 0, trackbsize, lastquat, -0.2,-0.7, 0.2, 0.7 );
  build_rotmatrix( R, lastquat );

  width_w0  = width_ini  + width_menu;
  height_w0 = height_ini + height_menu;
  width_w1=width_w0 - width_menu;
  height_w1=width_w1;

  /* Mutter-Fenster */
  glutInitWindowSize ( width_w0, height_w0 );
#if INX_MODE
  glutInitDisplayMode ( GLUT_INDEX | GLUT_DOUBLE );
#endif
#if TEX_MODE
  /* problems with xwd on sgi without GLUT_DEPTH */
  glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
#endif
  glutInit ( &argc, argv );
  sprintf (buffer, "Calculix Graphix");
  activWindow= w0 = glutCreateWindow ( buffer );
  glDisable ( GL_DEPTH_TEST );
  glMatrixMode ( GL_MODELVIEW );
  glutReshapeFunc ( reshape );
  glutMouseFunc ( MouseState );
  glutKeyboardFunc ( Keyboard );
  glutSpecialFunc ( specialKeyboard );
  glutVisibilityFunc ( WindowState );
  /* kein DITHERing (speed, Hoehenlinien)  */
  glDisable(GL_DITHER);
#if INX_MODE
  /* get the actual colormap and define the color-indexes for the main-window */
  getColormap();
  maxIndex= offset+steps-1;
  glShadeModel ( GL_FLAT );
#endif

  subsubmenu_animTune = glutCreateMenu( tuneAnimation );
  glutAddMenuEntry(" RESET TO 1.", 0);
  glutAddMenuEntry(" tune * 10", 1);
  glutAddMenuEntry(" tune *  5", 2);
  glutAddMenuEntry(" tune *  2", 3);
  glutAddMenuEntry(" tune /  2", 4);
  glutAddMenuEntry(" tune /  5", 5);
  glutAddMenuEntry(" tune / 10", 6);

  subsubmenu_animSteps = glutCreateMenu( stepsAnimation );
  glutAddMenuEntry("  4 Steps", 4);
  glutAddMenuEntry("  8 Steps", 8);
  glutAddMenuEntry(" 12 Steps", 12);
  glutAddMenuEntry(" 24 Steps", 24);
  glutAddMenuEntry(" 36 Steps", 36);
  glutAddMenuEntry(" 72 Steps", 72);

  subsubmenu_animPeriod = glutCreateMenu( newTimePerPeriod );
  glutAddMenuEntry(" Fastest     ", 1);
  glutAddMenuEntry(" 1,0 seconds ", 2);
  glutAddMenuEntry(" 1,2 seconds ", 3);
  glutAddMenuEntry(" 1,5 seconds ", 4);
  glutAddMenuEntry(" 2,0 seconds ", 5);
  glutAddMenuEntry(" 5,0 seconds ", 6);

  submenu_view = glutCreateMenu( selectView );
  glutAddMenuEntry("Show All Elements With Light", 1);
  glutAddMenuEntry("Show Bad Elements", 2);
  glutAddMenuEntry(" FILL  ", 3);
  glutAddMenuEntry(" LINES ", 4);
  glutAddMenuEntry(" DOTS ", 5);
  glutAddMenuEntry("Toggle Culling Back/Front", 6);
  glutAddMenuEntry("Toggle Model Edges", 7);
  glutAddMenuEntry("Toggle Element Edges", 8);
  glutAddMenuEntry("Toggle Surfaces/Volumes", 9);
  glutAddMenuEntry("Toggle Move-Z/Zoom", 10);
  glutAddMenuEntry("Toggle Background Color", 11);
  glutAddMenuEntry("Toggle Vector-Plot", 12);
  glutAddMenuEntry("Toggle Add-Displacement", 13);

  submenu_animate = glutCreateMenu( changeAnimation );
  glutAddMenuEntry("Start", 1);
  glutAddSubMenu  ("Tune-Value ", subsubmenu_animTune );
  glutAddSubMenu  ("Steps per Period", subsubmenu_animSteps );
  glutAddSubMenu  ("Time per Period ", subsubmenu_animPeriod );
  glutAddMenuEntry("Toggle Real Displacements", 2);
  glutAddMenuEntry("Toggle Static Model Edges", 3);
  glutAddMenuEntry("Toggle Static Element Edges", 4);
  glutAddMenuEntry("Toggle Dataset Sequence", 5);

  submenu_orientation = glutCreateMenu( orientModel );
  glutAddMenuEntry( "+x View     ", 1);
  glutAddMenuEntry( "-x View     ", 2);
  glutAddMenuEntry( "+y View     ", 3);
  glutAddMenuEntry( "-y View     ", 4);
  glutAddMenuEntry( "+z View     ", 5);
  glutAddMenuEntry( "-z View     ", 6);

  submenu_hardcopy = glutCreateMenu( markHardcopy );
  glutAddMenuEntry( "Tga-Hardcopy", 2);
  glutAddMenuEntry( "Ps-Hardcopy ", 1);
  glutAddMenuEntry( "Gif-Hardcopy", 4);
  glutAddMenuEntry( "Png-Hardcopy", 5);
  glutAddMenuEntry( "Start Recording Gif-Movie", 3);

  submenu_cut   = glutCreateMenu( selectCutNode   );
  glutAddMenuEntry( "Node 1 ", 1);
  glutAddMenuEntry( "Node 2 ", 2);
  glutAddMenuEntry( "Node 3 ", 3);
  glutAddMenuEntry( "Vector ", 5);
  glutAddMenuEntry( "Uncut   ", 4);

  submenu_graph = glutCreateMenu( selectGraphMode );
  glutAddMenuEntry( "Length ", 1);
  glutAddMenuEntry( "Datasets ", 2);
  glutAddMenuEntry( "Time ", 3);

  submenu_help = glutCreateMenu( showHelp );
  glutAddMenuEntry( "Quick Reference (cgx)", 1);
  glutAddMenuEntry( "Html Manual (cgx)", 2);
  glutAddMenuEntry( "Html Manual (ccx)", 3);
#ifdef AFLIB
  glutAddMenuEntry( "Html Manual (aflib)", 4);
#endif

  subsubmenu_parameter = glutCreateMenu( selectParameter );
  subsubmenu_entity = glutCreateMenu( selectEntityMenu );
  submenu_load = glutCreateMenu( selectDataset );


  list_elem_light = glGenLists((GLint)1);
  list_surf_light = glGenLists((GLint)1);
  list_elem_load = glGenLists((GLint)1);
  list_elem_elstress = glGenLists((GLint)1);
  list_surf_load = glGenLists((GLint)1);
  list_model_edges = glGenLists((GLint)1);
  list_surf_edges = glGenLists((GLint)1);
  list_elem_edges = glGenLists((GLint)1);

  /* Grafik-Fenster */
#if INX_MODE
  glutInitDisplayMode ( GLUT_INDEX | GLUT_DOUBLE | GLUT_DEPTH );
#endif
#if TEX_MODE
  glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
#endif
  activWindow= w1 = glutCreateSubWindow ( w0, width_menu*19/20, height_menu/10, width_ini, height_ini );
  glEnable ( GL_DEPTH_TEST );
  glDepthFunc(GL_LEQUAL);
  glFrontFace ( GL_CCW );
  glGetIntegerv (GL_MAX_EVAL_ORDER, &gl_max_eval_order);
  printf("GL_MAX_EVAL_ORDER:%d\n", gl_max_eval_order);
  /* Eventhandling Grafikfenster */
  glutMouseFunc ( MouseState );
  glutMotionFunc ( Mouse );
  glutKeyboardFunc ( Keyboard );
  glutSpecialFunc ( specialKeyboard );
  glutEntryFunc ( entryfunktion );
  glutPassiveMotionFunc ( Mouse );
  glDisable(GL_DITHER);
  glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
  glCullFace ( GL_BACK );
#if INX_MODE
  /* glutCopyColormap( w0 ); Geht auf cp0309 mit mesa nicht! */
  /* deshalb autonom belegen:  */
  for (i=0; i<256; i++)
  {
    glutSetColor( i, priv_cmap[i][0], priv_cmap[i][1], priv_cmap[i][2] );
  }
  initLightAndMaterial_index();
#endif
#if TEX_MODE
  initLightAndMaterial_rgb();
#endif


  /* Axenkreuz-Fenster-index im w1 fenster  */
#if INX_MODE
  glutInitDisplayMode ( GLUT_INDEX | GLUT_DOUBLE  );
#endif
#if TEX_MODE
  glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH  );
#endif
  activWindow= w2 = glutCreateSubWindow ( w1, 0, height_ini*0.9, width_ini/10, height_ini/10 );
  glutDisplayFunc ( DrawAxes );
  glDisable(GL_DITHER);

  if(inpformat=='b')
  {
    iniActionsFlag=1;
    glutIdleFunc ( idleFunction );
  }
  else if(inpformat=='x')
  {
    iniActionsFlag=2;
    glutIdleFunc ( idleFunction );
  }
  else
  {
    if(inpformat=='c') iniMeshData( datin, "ccx" );
    if(inpformat=='d') iniMeshData( datin, "duns2d" );
    if(inpformat=='i') iniMeshData( datin, "isaac2d" );
    if(inpformat=='j') iniMeshData( datin, "isaac3d" );
    if(inpformat=='e') iniMeshData( datin, "duns3d" );
    if(inpformat=='f') iniMeshData( datin, "foam" );
    if(inpformat=='m') iniMeshData( datin, "nas" );
    if(inpformat=='n') iniMeshData( datin, "ng" );
    if(inpformat=='s') iniMeshData( datin, "stl" );
    if(inpformat=='v') iniMeshData( datin, "frd" );

    /* calc additional entities only if the block was not jumped during read */
    for (i=0; i<anz->olc; i++)  if (lcase[i].loaded)
      calcDatasets( i, anz, node, lcase );

    if(anz->n>0)
    {
      if ( (set[setall].node = (int *)realloc((int *)set[setall].node, (anz->n+1) * sizeof(int))) == NULL )
        printf(" ERROR: malloc failed in set[%d]:%s\n\n", 0, set[setall].name);
      set[setall].anz_n=0;
      for (i=0; i<anz->n; i++) if(!node[node[i].nr].pflag)
      {
        set[setall].node[i]= node[i].nr;
        set[setall].anz_n++;
      }
      qsort( set[setall].node, set[setall].anz_n, sizeof(int), (void *)compareInt );
    }
    if(anz->e>0)
    {
      set[setall].anz_e=anz->e;
      if ( (set[setall].elem = (int *)realloc((int *)set[setall].elem, (anz->e+1) * sizeof(int))) == NULL )
        printf(" ERROR: malloc failed in set[%d]:%s\n\n", 0, set[setall].name);
      for (i=0; i<set[setall].anz_e; i++) set[setall].elem[i]= e_enqire[i].nr;
      qsort( set[setall].elem, set[setall].anz_e, sizeof(int), (void *)compareInt );
    }
    getScaleValues( setall, set, point, node, scale);
    scalNodes ( anz->n, node, scale );
    ConfigureAndShowWindow_Light();

    if(ccxfile[0]>0)
    {
      sprintf(buffer, "%s inp nom", ccxfile );
      pre_read(buffer);
    }
  }

  /* create the mainmenu */
  createDatasetEntries();

#if TEST
  printHash();
#endif

  glutMainLoop ();
  return(1);
}



void printHash()
{
  int i;
  FILE *handle[7];

  handle[0] = fopen ("hash.p", "w");
  if ( handle[0]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_sump; i++)
  {
    fprintf(handle[0], " sumASCI: %d indexes: %d\n", i, sumAsci->anzp[i]);
  }  
  handle[1] = fopen ("hash.l", "w");
  if ( handle[1]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_suml; i++)
  {
    fprintf(handle[1], " sumASCI: %d indexes: %d\n", i, sumAsci->anzl[i]);
  }  
  handle[2] = fopen ("hash.c", "w");
  if ( handle[2]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_sumc; i++)
  {
    fprintf(handle[2], " sumASCI: %d indexes: %d\n", i, sumAsci->anzc[i]);
  }  
  handle[3] = fopen ("hash.s", "w");
  if ( handle[3]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_sums; i++)
  {
    fprintf(handle[3], " sumASCI: %d indexes: %d\n", i, sumAsci->anzs[i]);
  }  
  handle[4] = fopen ("hash.b", "w");
  if ( handle[4]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_sumb; i++)
  {
    fprintf(handle[4], " sumASCI: %d indexes: %d\n", i, sumAsci->anzb[i]);
  }  
  handle[5] = fopen ("hash.a", "w");
  if ( handle[5]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_suma; i++)
  {
    fprintf(handle[5], " sumASCI: %d indexes: %d\n", i, sumAsci->anza[i]);
  }  
  handle[6] = fopen ("hash.S", "w");
  if ( handle[6]== NULL )
  {
    printf ("\nThe file could not be opened.\n\n"); 
    return;
  }
  for (i=0; i<sumAsci->max_sumS; i++)
  {
    fprintf(handle[6], " sumASCI: %d indexes: %d\n", i, sumAsci->anzS[i]);
  }  
}
