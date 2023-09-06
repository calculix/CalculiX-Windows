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

/*
Peter Heppel Jan 2016
   if keyword ndsb  (NoDeleteShellsBeams) is present in pre_read, 
   then DO NOT  delete elements in special sets created by readNG 

Check:
function check_line replaces check_tri3 in pre_proj for option "rot"
This has to be tested!
*/

#include <cgx.h>
#include <dirent.h>
#include <xwd.h>
#include <pthread.h>


#define TEST 0
#define NOTHREADING 0

/* choose the optimization-steps for the mesher */
#define MESHOPT_LENGTH 1
#define MESHOPT_ANGLE  0
#define MAX_PARAM_IN_BRECORD 100
#define     N_CLOSEST_TRI 150
#define     SMALL_TET_VOLUME 1.e-20
#define MAX_NR_OF_TETS_PER_ELEM 44
#define N_CLOSEST_TETS 200
extern int   cmaps;                         /* nr of colormap names */
extern char *cmap_names[];
extern char cmap_name[];         /* possible values: "classic", "viridis", "turbo", "inferno" */
extern int   col_maxc,col_minc;  /* colors of the regions with clipped colors (commands maxc,minc) */
extern int   defScalMethod;         /* method to display the scale */
extern int   entitycols;
extern Entitycol *entitycol;         /* predefined colors of entities */
extern char  datin[MAX_LINE_LENGTH];         /* Input-data-file */
extern char  buffer[MAX_LINE_LENGTH];
extern int   offset, maxIndex;                        /* offset+steps-1 = maxIndex */
extern int   basCol[3];                     /* color indexes due to basic colormap */
extern int   foregrndcol, backgrndcol;          /* default fore- and background color */
extern int   width_ini, height_ini; /* Grafig-Fensterbreite/hoehe */
extern int   width_menu, height_menu;
extern int   w0, w1, w_index, w_rgb, activWindow;
extern int   width_w0, height_w0;
extern int   width_w1, height_w1;
extern int   MouseMode;                                   /* status maustasten */
extern int   halfperiod;
extern double dtx, dty, dtz, drx, dry, drz, ds;                 /* Verschiebungen */
extern double anim_faktor;                            /* Scalierung der Amplitude */
extern char  drawMode;               /* protokoliert drawFunktion (Load=1,Light=2,Animate=3,preprocessor=4, vector=5)*/
extern char  automode;            /* set to 1 to perform various automatic actions as determining of divisions etc. during reading of command files */
extern char  allowSysFlag;           /* 1: allow the execution of system calls (sys command) */
extern char  autoDivFlag;                   /* The div command will set it to 0 and no auto-div is executed */
extern char  autoEltyFlag;                   /* The elty command will set it to 0 and no auto-elty is executed */
extern char  cullFlag;               /* 1: front face culling */
extern char  animFlag;
extern GLint   gl_max_eval_order;                         /* max order of NURBS */
extern GLdouble R[4][4];                                   /* Rotationsmatrix */
extern char  surfFlag;                /* zeichne nur Oberflaechenelemente (1), sonst (0)*/
extern char  modelEdgeFlag;                /* zeichne mit Modell-Ecken (1), sonst (0)*/
extern char  elemEdgeFlag;                 /* zeichne mit Surface Ecken (1), sonst (0)*/
extern char  frameFlag;                   /* mit (1) oder ohne Rahmen um das Grafikfenster */
extern char  sequenceFlag;                  /* 1: play a sequence of LC */
extern char  captionFlag;                   /* mit (1) oder ohne filename im Menufenster */
extern char  textFlag;                   /* mit (1) oder ohne text im Menufenster */
extern char  printFlag;                     /* printf 1:on 0:off */
extern char  frameFlag;               /* mit (1) oder ohne Rahmen um das Grafigfenster */
extern char  scalaFlag;                 /* mit (1) oder ohne scala und wertetexte */ 
extern char  addDispFlag;                    /* 0: original node-coordinates, 1: node-coordinates+displacements */
extern char  flipColorFlag;                 /* 0: high values use red, low use blue in scale; 1: flipped */
extern char  vectorFlag;                    /* 0: scalar plot, 1: vector plot */
extern int   frameSetFlag;               /* triggers frameSet() */
extern double dx ,dy;                                      /* Mauskoordinaten */
extern int   steps;                          /* Schrittweite der Farbscala */
extern double gtol;
extern int     ddiv;
extern double     dbias;
extern char  picture_caption[MAX_LINE_LENGTH];               /* Caption on window base line */
extern char  picture_text[MAX_LINE_LENGTH];
extern double v_scale;                                    /* scaling-factor for the vectors in the vector-plot */
extern char  browser[MAX_LINE_LENGTH];            /* html-browser */
extern char  psviewer[MAX_LINE_LENGTH];           /* (postscript) viewer */
extern char  viewformat[MAX_LINE_LENGTH];           /*  viewformat: ps,png */
extern char  helpfile[10][MAX_LINE_LENGTH];       /* help-file */
extern char  homepath[MAX_LINE_LENGTH];           /* path to the home dir */

extern int  cur_commandFile;                   /* >0: current open command file  */
int   stopped_commandFile=-1;               /* >0: stopped open command file  */
int   commandFiles=0;                  /* counts nr of open command files  */
CommandFile *commandFile;             /* stores informations of open command files */
extern char  delPntFlag;                    /* 1: deleted points exists */
extern char  delShapeFlag;                  /* 1: deleted shapes exists */
extern char  delLineFlag;                   /* 1: deleted lines exists */
extern char  delLcmbFlag;                   /* 1: deleted lcmbs exists */
extern char  delSurfFlag;                   /* 1: deleted surfs exists */
extern char  delBodyFlag;                   /* 1: deleted bodys exists */
extern char  delNursFlag;
extern char  delSetFlag;                  /* 1: deleted sets exists */
extern char  movieFlag;                     /* >0: save sequence of gif pictures */
extern int  movieFrames;                     
extern char  movieCommandFile[MAX_LINE_LENGTH];        /* stores the file name of a command file which will be executed after the movie is created (frames option only) */
extern char  stopFlag;                      /* stop/start animation */
extern char  backgroundFlag;                /* if 1 no graphic updates are done during reading of command files (readfbd).
                                                 This speeds significantly up the execution of the commands. */
char inpformatbuffer;
extern char  inpformat;                     /* defines the start-up mode of cgx */
extern int     read_mode;
extern int     setall;
extern int   animList;
extern int   lcase_animList;

extern int     elemMat[MAX_MATERIALS];      /*  Material Numbers, Number of Materials stored in elemMat[0]  */

extern int       nasMpc;                                       /* 1: areampc generates mpcs; 0: rbes with optional heat-expansion-coefficient */
extern double    nasRbeHec; 

/* for rgb mode */
extern Colours   *colRgb;

/* for index mode */
extern int       *colNr;

extern Scale     scale[1];
extern Elements  *e_enqire;     /* elem-array by elem-number instead of elem[index]... */
extern Summen    anz[1];
extern Edges     *edge;
extern Nodes     *node;
extern Datasets *lcase;
extern Faces     *face;
extern NodeBlocks *nBlock;
extern BGpicture *bgpicture;

extern Meshp meshp;

extern Alias     *alias;
extern Sets      *set;
extern Shapes    *shape;
extern Materials *material; 
extern Psets     *pset;
extern Values    *value;
extern Points    *point;
extern Lines     *line;
extern Lcmb      *lcmb;
extern Gsur      *surf;
extern Gbod      *body;
extern Nurbl     *nurbl;
extern Nurbs     *nurbs;
extern Materials *material; 
extern Amplitudes *amplitude; 
extern SumGeo    anzGeo[1];
extern SumAsci   sumAsci[1];
extern Texts     *ntext;

/* nr of graph 2d plot */
extern int graph_Nr;
extern int graph_on; 

/* nr of hardcopies */
extern int psNr, tgaNr, gifNr, pngNr;


extern Eqal eqal;

/* additional entities */
extern char **valuestack;
extern int valuestack_ptr, valuestackFlag;
extern OpenSets   openSets[1];
extern SpecialSet specialset[1];
extern int  set_bsur;


Lchar     lchar[1]={{'0','0','D','L','C','A','B','Q','N','A','H'}};      /* first letter in the entity names */

extern int  cur_entity;                                       /* aktive entity (component) */
extern int  cur_lc;
extern void       *glut_font[];  /* glut fonts */
extern int       legend_font;                         /* active font for the legend */
extern int       draw_font;                         /* active font for the annotation of entities */
extern int       menu_font;                         /* active font for the menu */

/* the copied node-sets which have to be filled with values from new loaded Datasets */
extern CopiedNodeSets copiedNodeSets[1];
extern char **parameter;

extern sem_t  sem_n;
sem_t   sem_map3d;

int       nodeCsys=0;                        /*  Number of coordinate systems (nastran)  */
int       nodeCsysSet[MAX_MATERIALS];        /*  setNr  */
int       nodeCsysNr[MAX_MATERIALS];         /*  csysNr  */

/* mpc generation between incompatible element-types for connected bodies */
int sum_equSets=0, *depSet=NULL, *indSet=NULL;


/* returns node-Index if known, or -(Index+10) of a unused node or if none is unused -1  */
int getNodNr(Summen *anz, Nodes *node, int nodnr )
{
  //int pfree;
  //printf("nodnr:%d anz->nmax:%d \n", nodnr,anz->nmax);
  if(nodnr>anz->nmax) return(-1);
  //printf("nodindx:%d anz->n:%d \n", node[nodnr].indx,anz->n);
  if(node[nodnr].indx<0) return(-1);
  if(node[nodnr].indx>anz->n) return(-1);

  if( node[node[nodnr].indx].nr == nodnr )
  {
    /* unused node ?  */
    //if( node[nodnr].pflag == -1) return(pfree=-node[nodnr].indx-10);
    if( node[nodnr].pflag == -1) return(-node[nodnr].indx-10);
    else return(node[nodnr].indx);
  }
  return(-1);
}



/* returns alias-Index if known, or if not -1  */
int getAliasNr(char *name)
{
  int i, n, length, pfree, sum;

  if(!anzGeo->alias) return(-1);

  i=length=sum=0; 
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);

  if(sum<0)
  {
    printf ("ERROR: Illegal name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_suma)
  {
   for (i=0; i<sumAsci->anza[sum]; i++)
   {
    if(( alias[sumAsci->aindx[sum][i]].name != (char *)NULL ) && (strlen(alias[sumAsci->aindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(alias[sumAsci->aindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->aindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, alias[sumAsci->pindx[sum][i]].name, sumAsci->aindx[sum][i]);
      */
    }
   }
  }
  else
  {
    if(printFlag) printf(" WARNING alias:%s not found in hash-table\n", name);
     for (i=0; i<anzGeo->alias; i++) if( alias[i].name != (char *)NULL )
      if((compare( alias[i].name, name, length)==length) && (sword( alias[i].name, buffer)==length))
        return(i);
  }
  return(pfree);
}


/* returns value-Index if known, or -(Index+10) of a deleted value or if none is deleted -1  */
int getValuNr(char *name)
{
  int i, n, length, pfree, sum;

  if(!anz->v) return(-1);

  i=length=sum=0; 
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal value name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumv)
  {
   for (i=0; i<sumAsci->anzv[sum]; i++)
   {
    if(( value[sumAsci->vindx[sum][i]].name != (char *)NULL ) && (strlen(value[sumAsci->vindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(value[sumAsci->vindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->vindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, value[sumAsci->vindx[sum][i]].name, sumAsci->vindx[sum][i]);
      */
    }
   }
  }
  else 
  {
    if(printFlag) printf(" WARNING value:%s not found in hash-table\n", name);
    for (i=0; i<anz->v; i++) if( value[i].name != (char *)NULL )
      if((compare( value[i].name, name, length)==length) && (sword( value[i].name, buffer)==length))
        return(i);
  }

  /* if not found  look for free cells */
  for (i=0; i<anz->v; i++)
  {
    /* do we have a "deleted" point for use?  */
    if( value[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  return(pfree);
}


/* returns pnt-Index if known, or -(Index+10) of a deleted pnt or if none is deleted -1  */
int getPntNr(char *name)
{
  int i, n, length, pfree, sum;

  if(!anzGeo->p) return(-1);
  if(name==(char *)NULL) return(-1);

  i=length=sum=0; 
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Point name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sump)
  {
   for (i=0; i<sumAsci->anzp[sum]; i++)
   {
    if(( point[sumAsci->pindx[sum][i]].name != (char *)NULL ) && (strlen(point[sumAsci->pindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(point[sumAsci->pindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->pindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, point[sumAsci->pindx[sum][i]].name, sumAsci->pindx[sum][i]);
      */
    }
   }
  }
  else 
  {
    if(printFlag) printf(" WARNING point:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->p; i++) if( point[i].name != (char *)NULL )
      if((compare( point[i].name, name, length)==length) && (sword( point[i].name, buffer)==length))
        return(i);
  }

  /* if not found  look for free cells */
  if (delPntFlag)
  for (i=0; i<anzGeo->p; i++)
  {
    /* do we have a "deleted" point for use?  */
    if( point[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delPntFlag=0; /* only if "deleted" was no more found */
  return(pfree);
}


/* returns shape-Index if known, or -(Index+10) of a deleted shape or if none is deleted -1  */
int getShapeNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anzGeo->sh) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);

  /*
  printf ("in getShapeNr anzsh[%d]: %d\n", sum, sumAsci->anzsh[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal shape name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumsh)
  {
   for (i=0; i<sumAsci->anzsh[sum]; i++)
   {
    if(( shape[sumAsci->shindx[sum][i]].name != (char *)NULL ) && (strlen(shape[sumAsci->shindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(shape[sumAsci->shindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->shindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, shape[sumAsci->shindx[sum][i]].name, sumAsci->shindx[sum][i]);
      */
    }   
   } 
  }
  else
  {
    if(printFlag) printf(" WARNING shape:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->sh; i++)
    {
      if( shape[i].name != (char *)NULL )
      {
        if((compare( shape[i].name, name, length)==length) && (sword( shape[i].name, buffer)==length))
          return(i);
      }
    }
  }

  /* if not found look for free cells */
  if (delShapeFlag)
  for (i=0; i<anzGeo->sh; i++)
  {
    /* do we have a "deleted" shape for use?  */
    if( shape[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  } 
  delShapeFlag=0;
  return(pfree);
}


/* returns line-Index if known, or -(Index+10) of a deleted line or if none is deleted -1  */
int getLineNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anzGeo->l) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);

  /*
  printf ("in getLineNr anzl[%d]: %d\n", sum, sumAsci->anzl[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal line name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_suml)
  {
   for (i=0; i<sumAsci->anzl[sum]; i++)
   {
    if(( line[sumAsci->lindx[sum][i]].name != (char *)NULL ) && (strlen(line[sumAsci->lindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(line[sumAsci->lindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->lindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, line[sumAsci->lindx[sum][i]].name, sumAsci->lindx[sum][i]);
      */
    }   
   } 
  }
  else
  {
    if(printFlag) printf(" WARNING line:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->l; i++)
    {
      if( line[i].name != (char *)NULL )
      {
        if((compare( line[i].name, name, length)==length) && (sword( line[i].name, buffer)==length))
          return(i);
      }
    }
  }

  /* if not found look for free cells */
  if (delLineFlag)
  for (i=0; i<anzGeo->l; i++)
  {
    /* do we have a "deleted" line for use?  */
    if( line[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  } 
  delLineFlag=0;
  return(pfree);
}


/* returns line-Index if known, or -(Index+10) of a deleted line or if none is deleted -1  */
int getLcmbNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anzGeo->c) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  /* printf ("in getLcmbNr sum:%d max_sumc:%d anzGeo->c:%d\n", sum, sumAsci->max_sumc, anzGeo->c); */
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Lcmb name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumc)
  {
   /* printf ("in getLcmbNr anzc[%d]:%d \n", sum, sumAsci->anzc[sum]); */
   for (i=0; i<sumAsci->anzc[sum]; i++)
   {
    if(( lcmb[sumAsci->cindx[sum][i]].name != (char *)NULL ) && (strlen(lcmb[sumAsci->cindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(lcmb[sumAsci->cindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->cindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, lcmb[sumAsci->cindx[sum][i]].name, sumAsci->cindx[sum][i]);
      */
    }
   }    
  }
  else 
  {
    if(printFlag) printf(" WARNING lcmb:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->c; i++) if( lcmb[i].name != (char *)NULL )
      if((compare( lcmb[i].name, name, length)==length) && (sword( lcmb[i].name, buffer)==length))
        return(i);
  }

  /* if not found look for free cells */
  if (delLcmbFlag)
  for (i=0; i<anzGeo->c; i++)
  {
    /* do we have a "deleted" lcmb for use?  */
    if(( lcmb[i].name == (char *)NULL )&& ( lcmb[i].o == NULL )&& ( lcmb[i].l == NULL ))
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delLcmbFlag=0;
  return(pfree);
}


/* returns surf-Index if known, or -(Index+10) of a deleted surf or if none is deleted -1  */
int getSurfNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anzGeo->s) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  /*
  printf ("in getSurfNr anzs[%d]: %d\n", sum, sumAsci->anzs[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal Surf name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sums)
  {
   for (i=0; i<sumAsci->anzs[sum]; i++)
   {
    if(( surf[sumAsci->sindx[sum][i]].name != (char *)NULL ) && (strlen(surf[sumAsci->sindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(surf[sumAsci->sindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->sindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, surf[sumAsci->sindx[sum][i]].name, sumAsci->sindx[sum][i]);
      */
    }
   } 
  }
  else
  {
    if(printFlag) printf(" WARNING surf:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->s; i++) if( surf[i].name != (char *)NULL )
    {
      if((compare( surf[i].name, name, length)==length) && (sword( surf[i].name, buffer)==length)) return(i);
    }
  }

  /* if not found look for free cells */
  if (delSurfFlag)
  for (i=0; i<anzGeo->s; i++)
  {
    /* do we have a "deleted" surf for use?  */
    if( surf[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delSurfFlag=0;
  return(pfree);
}

/* returns body-Index if known, or -(Index+10) of a deleted body or if none is deleted -1  */
int getBodyNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anzGeo->b) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  /*
  printf ("in getBodyNr anzb[%d]: %d\n", sum, sumAsci->anzb[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal Body name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumb)
  {
   for (i=0; i<sumAsci->anzb[sum]; i++)
   {
    if(( body[sumAsci->bindx[sum][i]].name != (char *)NULL ) && (strlen(body[sumAsci->bindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(body[sumAsci->bindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->bindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, body[sumAsci->bindx[sum][i]].name, sumAsci->bindx[sum][i]);
      */
    }
   }   
  }
  else 
  {
    if(printFlag) printf(" WARNING body:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->b; i++) if( body[i].name != (char *)NULL )
      if((compare( body[i].name, name, length)==length) && (sword( body[i].name, buffer)==length))
        return(i);
  }

  /* if not found check all and look for free cells */
  /* check all is not active, should not be necessary */
  if (delBodyFlag)
  for (i=0; i<anzGeo->b; i++)
  {
    /* do we have a "deleted" body for use?  */
    if( body[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delBodyFlag=0;
  return(pfree);
}


/* returns Nurs-Index if known, or -(Index+10) of a deleted body or if none is deleted -1  */
int getNursNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anzGeo->nurs) return(-1);
  if(compareStrings(name, "BLEND")>0) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') {  sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Nurs name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }

  if(sum<=sumAsci->max_sumS)
  {
   for (i=0; i<sumAsci->anzS[sum]; i++)
   {
    if(( nurbs[sumAsci->Sindx[sum][i]].name != (char *)NULL ) && (strlen(nurbs[sumAsci->Sindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(nurbs[sumAsci->Sindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->Sindx[sum][i]);
	}
      }
      //printf(" name:%s nam:%s indx:%d\n", name, nurbs[sumAsci->Sindx[sum][i]].name, sumAsci->Sindx[sum][i]);
    }
   }
  }
  else 
  {
   if(printFlag)
     printf("WARNING NURS:%s not found in hash-table\n", name);
   for (i=0; i<anzGeo->nurs; i++)
   {
     // printf(" nurbsname:%d %d\n", i, nurbs[i].name);
    if( nurbs[i].name != (char *)NULL )
    {
      //printf("nurs:%s length:%d name:%s length:%d\n", nurbs[i].name, strlen(nurbs[i].name), name, length);
      if((compare( nurbs[i].name, name, length)==length) && (sword( nurbs[i].name, buffer)==length))
        return(i);
    }
   }
  }

  /* if not found check all and look for free cells */
  /* check all is not active, should not be necessary */
  if (delNursFlag)
  for (i=0; i<anzGeo->nurs; i++)
  {
    /* do we have a "deleted" nurbs for use?  */
    if( nurbs[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delNursFlag=0;
  return(pfree);
}

/* returns nurl-Index if known, or -(Index+10) of a deleted nurl or if none is deleted -1  */
int getNurlNr(char *name)
{
  int i, length, pfree;

  if(!anzGeo->nurl) return(-1);

  i=0;
  pfree=-1;
  length=sword( name, buffer );
  if(!length) return(-1);

  for (i=0; i<anzGeo->nurl; i++)
  {
    if( nurbl[i].name != (char *)NULL )
    {
      if( ( compare( nurbl[i].name, name, length)==length) && ( sword( nurbl[i].name, buffer)==length) )
        return(i);
      if( nurbl[i].name == (char *)NULL )
        pfree=-i-10;
    }
  }
  return(pfree);
}



/* returns set-Index if known, or -(Index+10) of a deleted set or if none is deleted -1  */
int getSetNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anz->sets) return(-1);
  if(name== (char *)NULL) return(-1);

  i=length=sum=0;
  pfree=-1;

  while((name[length]!='\0')&&(length<MAX_LINE_LENGTH)) { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Set name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumse)
  {
   for (i=0; i<sumAsci->anzse[sum]; i++)
   {
    if(( set[sumAsci->seindx[sum][i]].name != (char *)NULL ) && (strlen(set[sumAsci->seindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(set[sumAsci->seindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->seindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, set[sumAsci->seindx[sum][i]].name, sumAsci->seindx[sum][i]);
      */
    }
   }    
  }

  if(printFlag) printf(" WARNING set:%s not found in hash-table\n", name);
  for (i=0; i<anz->sets; i++)
  {
    if (set[i].name!= (char *)NULL)
    {
      if ((compare( set[i].name, name, length)==length) && (sword( set[i].name, buffer)==length))
        return(i);
    }
  }

  /* set not found, check if the name could be an set-index */
  if (checkIfNumber(name)>0)
  {
    /* the user has specified the set-index */
    n=atoi(name);
    if(n<=anz->sets)
    {
      for (i=0; i<anz->sets; i++) if(set[i].index==n)
      {
        printf (" index %s points to %s\n", name, set[i].name);
        return (i);
      }
    }
  }

  /* if not found check all and look for free cells */
  /* check all is not active, should not be necessary */
  if (delSetFlag)
  for (i=0; i<anz->sets; i++)
  {
    /* do we have a "deleted" set for use?  */
    if( set[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delSetFlag=0;
  return(pfree);
}




/* search an element in an sorted integer-array */
/* return -1 if not found or index if found */
int getIndex(int **ipnt, int n, int x0 )
{
  int i=0,ii,m,n1,n2;
  int *x;

  x=*ipnt;
  /* if x0 is lower than the first elem */
  if((n==0)||(x0<x[0])) return(-1);

  /* if x0 is higher than the last elem */
  else if(x0>x[n-1]) return(-1);

  else
  {
    /* search the intersection */
    n1=0;                              
    n2=n;                            
    for(ii=0; ii<n; ii++)
    {                     
      m=(n2+n1)/2;                      
      if(x0>= x[m] ) n1=m;              
      if(x0 < x[m] ) n2=m;              
      if((n2-n1) == 1) break;           
    }                                 
    i=n1;
#if TEST
    printf("i:%d x:%d x0:%d x++:%d\n", i,x[i],x0,x[i+1]); 
#endif

    if (x0!=x[i])
    {
      /* element not found */
      return(-1);
    }
  }

#if TEST
  for(ii=0; ii<n; ii++)
  {
    printf("i:%d x:%d \n", ii,x[ii]); 
  }
#endif
 
#if TEST
  printf("b:%d\n", *(*ipnt)); 
#endif
  return(i);
}



void delSet( char *setname)
{
  int i, j;
  int   setNr;

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    if(printFlag) printf (" delSet: set:%s does not exist\n", setname);
    return;
  }
  /* suche abhaengige entities */
  if( set[setNr].type )
  {
    /* durchsuche alle linien   */
    for(i=0; i<anzGeo->l; i++)
    {
      if (( line[i].name != (char *)NULL)&&(line[i].typ=='s')&&(setNr==line[i].trk))
      {
        printf (" ERROR: trk removed, Line will be straight\n");
        line[i].trk=-1;
        line[i].typ=' ';
      }
    }
  }
  /* durchsuche alle uebrigen sets und entferne den set  */
  for(i=0; i<anz->sets; i++)
  {
    if((set[i].name!=(char *)NULL)&&(set[i].name[0]!='-')&&( getIndex(&set[i].set,set[i].anz_se, setNr) >-1))
    {
      setr( i, "r", setNr);
    }
  }
  /* durchsuche alle psets und entferne den set   */
  j=0; for(i=0; i<anzGeo->psets; i++)
  {
    if (setNr!=pset[i].nr)
    {
      if(i!=j) {
        pset[j].nr=pset[i].nr;
        strcpy(pset[j].type,pset[i].type);
        pset[j].col=pset[i].col;
      }
      j++;
    }
  }
  anzGeo->psets=j;

  /* der set wird wieder frei gegeben */
  delSetFlag=1;
  if(set[setNr].anz_elf)
    for(i=0; i<set[setNr].anz_elf; i++)
      if(set[setNr].elf[i].n) free(set[setNr].elf[i].v);  
    free(set[setNr].name);
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
    set[setNr].name=(char *)NULL;
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

/*------------------------------------------------------------------*/
/* einen set als offen markieren                                    */
/*------------------------------------------------------------------*/

int seto( char *setname )
{
  int   i,setNr;

  operateAlias( setname, "se" );

  setNr=getSetNr(setname);

  if (strlen(setname)<=0)
  {
    /* list all open sets */
    for (i=0; i<anz->sets; i++)
    {
      if( set[i].name != (char *)NULL )
      {
        if ((!set[i].type)&&(set[i].flag=='o'))
          printf ("%s stat:%c n:%d e:%d f:%d p:%d l:%d c:%d s:%d b:%d L:%d S:%d se:%d sh:%d\n", set[i].name, set[i].flag, set[i].anz_n, set[i].anz_e, set[i].anz_f, set[i].anz_p, set[i].anz_l, set[i].anz_c, set[i].anz_s, set[i].anz_b, set[i].anz_nurl, set[i].anz_nurs, set[i].anz_se, set[i].anz_sh);
      }
    }
    return(-1);
  }

  if (setNr==-1)       /* open a new set */
  {
    if ((set = (Sets *)realloc( (Sets *)set, (anz->sets+1)*sizeof(Sets)) ) == NULL )
    {
      printf(" ERROR: realloc failure in seto, set:%s not installed\n\n", setname);
      return(-1);
    }
    setNr= anz->sets;
    anz->sets++;

    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    hashSet( sumAsci, setname, setNr );
    if(printFlag) printf (" create and open set:%s\n", set[setNr].name);
    set[setNr].flag='o';
    set[setNr].type=0;
    set[setNr].etyp=0;
    set[setNr].eattr=0;
    set[setNr].eseq=0;
    if((set[setNr].valu= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].node= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elem= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].face= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elf= (Elfaces *)malloc(sizeof(Elfaces))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].pnt= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].line= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].lcmb= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].surf= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].body= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurl= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurs= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].set= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].shp= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    set[setNr].lock = 0;
    set[setNr].index = -1;
    set[setNr].material = -1;
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
    set[setNr].eparm=(char *)NULL;
  }
  else if (setNr<-1)    /* replace a deleted set */
  {
    setNr=-(setNr+10);
    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    hashSet( sumAsci, setname, setNr );
    set[setNr].flag='o';
    set[setNr].type=0;
    set[setNr].lock = 0;
    if(printFlag) printf (" open set:%s\n", set[setNr].name);
  }
  else
  {
    if (set[setNr].flag=='o')
    {
      printf (" set:%s is already open\n", set[setNr].name);
      set[setNr].lock = 0;
      return(setNr);
    }
    else
    {
      if(printFlag) printf (" open set:%s\n", set[setNr].name);
      set[setNr].lock = 0;
      set[setNr].flag='o';
    }
  }

  /* add the set to openSets */
  openSets->nr++;
  if( (openSets->set = (int *)realloc( (int *)openSets->set, openSets->nr*sizeof(int))) == NULL )
  { printf(" ERROR: realloc failed in seto\n\n"); return(-1); }
  openSets->set[ openSets->nr-1 ] = setNr;
  if(printFlag) printf("seto: openSets->nr:%d openSets->set:%d set:%s\n", openSets->nr, openSets->set[openSets->nr-1],set[setNr].name);
  return(setNr);
}


/*------------------------------------------------------------------*/
/* einen set schliessen                                             */
/*------------------------------------------------------------------*/

void setc( char *setname )
{
  int   setNr, i, n, flag=0;
  int *puf=NULL;

  operateAlias( setname, "se" );
  if (strlen(setname)>0) setNr=getSetNr(setname);
  else if(openSets->nr>0) setNr=openSets->set[openSets->nr-1];
  else { printf("no open set\n"); return; }

  if (setNr<0)
  {
    printf (" setc: set:%s does not exist\n", setname);
    return;
  }
  else
  {
    if (set[setNr].flag=='c')
    {
      printf (" set:%s is already closed\n", set[setNr].name);
    }
    else
    {
      if(printFlag) printf (" close set:%s\n", set[setNr].name);
      set[setNr].flag='c';

      /* remove the set from openSets */
      /* puffer allocieren  */
      if ( (puf = (int *)malloc( (openSets->nr+1)*sizeof(int))) == NULL )
      printf(" ERROR: malloc failed in setc\n\n");

      n=0;
      for ( i=0; i<openSets->nr; i++)
      {
        if ( openSets->set[i]!=setNr )
        {
          puf[n] = openSets->set[i];
          n++;
        }
        else
          flag=1;
      }
 
      if (flag)
      {
        openSets->nr--;
        if ((openSets->set = (int *)realloc( (int *)openSets->set, (openSets->nr+1)*sizeof(int)) ) == NULL)
        { printf(" ERROR: realloc failed in setc\n\n"); free(puf); return; }

        for ( i=0; i<openSets->nr; i++)
        {
          openSets->set[i] = puf[i];
        }
      }
      free(puf);
      if(printFlag) printf("setc: openSets->nr:%d openSets->set:%d set:%s\n", openSets->nr, setNr, set[setNr].name); 
    }
  }
}


/*------------------------------------------------------------------*/
/* entity einem Set zuordnen                                        */
/*------------------------------------------------------------------*/


int hashSet( SumAsci *sumAsci, char *name, int nr)
{
  int i=0,j=0, n;
  int sum=0; 

  while(name[i]!='\0') { sum+=name[i]*(++j); i++; }

  /* check if sum is higher as the allocated value */
  /* if not look for a free entry */
  if(sum>sumAsci->max_sumse)
  {
    if ((sumAsci->anzse=(int *)realloc( (int *)sumAsci->anzse, (sum+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure in hashSet(), set:%s not included\n\n", name); return(-1); }
    if ((sumAsci->seindx=(int **)realloc( (int **)sumAsci->seindx, (sum+1)*sizeof(int *)) ) == NULL )
    { printf("\n\nERROR: realloc failure in hashSet(), set:%s not included\n\n", name); return(-1); }
    for(i=sumAsci->max_sumse+1; i<=sum; i++) { sumAsci->anzse[i]=0; sumAsci->seindx[i]=NULL; }
    sumAsci->max_sumse=sum;
  }
  else
  {
    if (delSetFlag)
    for (i=0; i<sumAsci->anzse[sum]; i++) 
    {
      n=sumAsci->seindx[sum][i];
      if( set[n].name == (char *)NULL )
      {
        /* already existing space to fill */
        sumAsci->seindx[sum][i]=nr;
        return(sum);
      }
    }
  }

  /* alloc of a new entry in the hash table */
  if ((sumAsci->seindx[sum] =(int *)realloc( (int *)sumAsci->seindx[sum], (sumAsci->anzse[sum]+1)*sizeof(int)) ) == NULL )
  { printf("\n\nERROR: realloc failure in hashSet(), set:%s not included\n\n", name); return(-1); }

  sumAsci->seindx[sum][sumAsci->anzse[sum]] = nr;
  sumAsci->anzse[sum]++;
  return(sum);
}



int pre_seta( char *string, char *type, char *name)
{
  int i, setNr;
  int n=0;
  int number;
  char setname[MAX_LINE_LENGTH]; /* string is not changeable. Therefore a new char is necessary */

  /* remove blanks and other illegal chars*/
  for(i=0;i<strlen(string); i++) if(string[i]>(char)0x20) { setname[n]=string[i]; n++; }
  if(!n) return(-1);
  setname[n]='\0';

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  number=0;

  if (setNr==-1)
  {
    if ((set = (Sets *)realloc( (Sets *)set, (anz->sets+2)*sizeof(Sets)) ) == NULL )
    {
      printf(" ERROR: realloc failure in pre_seta, set:%s not installed\n\n", setname);
      return(-1);
    }
    setNr= anz->sets;
    anz->sets++;

    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    hashSet( sumAsci, setname, setNr );
    if(printFlag) printf (" create set:%s\n", set[setNr].name);
    set[setNr].flag='c';
    if ( type[1] == 's' ) set[setNr].type=1;
    else                  set[setNr].type=0;
    set[setNr].etyp=0;
    set[setNr].eattr=0;
    set[setNr].eseq=0;
    if((set[setNr].valu= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].node= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elem= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].face= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elf= (Elfaces *)malloc(sizeof(Elfaces))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].pnt= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].line= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].lcmb= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].surf= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].body= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurl= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurs= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].set= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].shp= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    set[setNr].lock=0;
    set[setNr].index = -1;
    set[setNr].material = -1;
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
    set[setNr].eparm=(char *)NULL;
  }
  else if (setNr<-1)    /* replace a deleted set */
  {
    setNr=-(setNr+10);
    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    if ( type[1] == 's' ) set[setNr].type=1;
    else                  set[setNr].type=0;
    hashSet( sumAsci, setname, setNr );
    if((set[setNr].valu= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].node= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elem= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].face= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elf= (Elfaces *)malloc(sizeof(Elfaces))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].pnt= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].line= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].lcmb= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].surf= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].body= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurl= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurs= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].set= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].shp= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    set[setNr].lock=0;
    set[setNr].index = -1;
    set[setNr].material = -1;
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
    set[setNr].eparm=(char *)NULL;
  }

  /* determine the index of the entity */
  if (( type[0] == 's' )&&( type[1] == 'e' ))  number=getSetNr(name);
  else if (( type[0] == 's' )&&( type[1] == 'h' ))  number=getShapeNr(name);
  else if ( type[0] == 'n' )                   if(checkIfNumber(name)) number=atoi(name); else return(-1);
  else if ( type[0] == 'e' )                   if(checkIfNumber(name)) number=atoi(name); else return(-1);
  else if ( type[0] == 'f' )                   if(checkIfNumber(name)) number=atoi(name); else return(-1);
  else if ( type[0] == 'v' )                   number=getValuNr(name);
  else if ( type[0] == 'p' )                   number=getPntNr(name);
  else if ( type[0] == 'l' )                   number=getLineNr(name);
  else if ( type[0] == 'c' )                   number=getLcmbNr(name);
  else if ( type[0] == 's' )                   number=getSurfNr(name);
  else if ( type[0] == 'b' )                   number=getBodyNr(name);
  else if ( type[0] == 'L' )                   number=getNurlNr(name);
  else if ( type[0] == 'S' )                   number=getNursNr(name);
  else if ( type[0] == 'r' )                   number=getSetNr(name);
  else if ( type[0] == 'i' ) 
  {
    if(printFlag) printf (" set initialized\n");
    return(setNr);
  }
  else
  {
    errMsg ("ERROR: in seta type:%s not recognized\n", type);
    return(-1);
  }
  if(number<0)
  {
    if(printFlag) errMsg ("WARNING: in seta entity:%s not known\n", name);
    return(-1);
  }

  /* add to the set */
  if( seta(setNr, type, number)<0 )
  {
    errMsg ("ERROR: seta failed\n", name);
    return(-1);
  }
  else
    return(setNr);
}



int rnam( int setNr, char *new_name)
{
  if(strlen(set[setNr].name)<strlen(new_name))
  {
    if((set[setNr].name= (char *)realloc((char *)set[setNr].name, (strlen(new_name)+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed\n\n" ); return(-1); }
  }
  strcpy(set[setNr].name, new_name );
  hashSet( sumAsci,set[setNr].name, setNr );
  return(setNr);
}



int seta( int setNr, char *type, int number)
{
  int k;
  if( set[setNr].lock == 1 ) return(-1);
  if( set[setNr].name == (char *)NULL )
  {
    errMsg(" ERROR: setNr:%d is undefined\n", setNr);
    return(-1);
  }
  if( number<0)
  {
    errMsg(" ERROR in seta: A negative entity-index:%d was used in set %s\n", number,set[setNr].name);
    return(-1);
  }
  //printf("type:%c set:%s\n", type[0], set[setNr].name );

  /* check if item is known and if its already member of the set */
  if ( type[0] == 'r' )
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: set:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_se= iinsert(&set[setNr].set, set[setNr].anz_se, number);
    set[number].anz_se= iinsert(&set[number].set, set[number].anz_se, setNr);
  }
  else if ( type[0] == 'v' )
  {
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_v=iinsert(&set[setNr].valu, set[setNr].anz_v, number);
  }
  else if (( type[0] == 's' )&&( type[1] == 'e' ))
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: set:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_se= iinsert(&set[setNr].set, set[setNr].anz_se, number);
  }
  else if (( type[0] == 'h' )||(( type[0] == 's' )&&( type[1] == 'h' )))
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: shape:%d in set %s does not exist\n", number,set[setNr].name ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_sh= iinsert(&set[setNr].shp, set[setNr].anz_sh, number);
  }
  else if ( type[0] == 'n' )
  {
    if(!anz->n) return(1);
    if((number<anz->nmin)|| (number>anz->nmax)||(node[number].indx<0) || (number!=node[node[number].indx].nr)||(node[number].pflag!=0))    
      {  printf(" ERROR in seta: node:%d does not exist and can not be placed in set %s, min:%d max:%d\n", number,set[setNr].name, anz->nmin, anz->nmax); return(-1); }

    if(set[setNr].type==1) /* seqa */
    {
      /*
      for (n=0; n<set[setNr].anz_n; n++)
      {
        if( set[setNr].node[n] == number )
        { if(printFlag) printf(" ERROR in seta: node already stored in set %s\n",set[setNr].name); return(-1); }
      }
      */
      set[setNr].anz_n++;
      if( (set[setNr].node = (int *)realloc( (int *)set[setNr].node, set[setNr].anz_n*sizeof(int))) == NULL )
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); return(-1); }
      else
      {
        if(printFlag) printf(" set[%d]:%s reallocated and node %d added\n"
            , setNr, set[setNr].name, number );
      }
      set[setNr].node[ (set[setNr].anz_n-1) ] = number;
    }
    else
      set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n, number);
  }
  else if ( type[0] == 'e' )
  {
    if(!anz->e) return(1);
    if ((number<anz->emin)|| (number>anz->emax) || (e_enqire[number].type==0))
      { printf(" ERROR in seta: set %s, elem:%d does not exist, min:%d max:%d\n", set[setNr].name, number, anz->emin, anz->emax ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e, number);
  }
  else if ( type[0] == 'f' )
  {
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_f=iinsert(&set[setNr].face, set[setNr].anz_f, number);
  }
  else if ( type[0] == 'p' )
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: set %s, pntNr:%d does not exist\n", set[setNr].name, number ); return(-1); }

    if(set[setNr].type==1) /* seqa */
    {
      /*
      for (n=0; n<set[setNr].anz_p; n++)
      {
        if( set[setNr].pnt[n] == number )
        { if(printFlag) printf(" ERROR in seta: pnt already stored in set %s\n", set[setNr].name); return(-1); }
      }
      */
      set[setNr].anz_p++;
      if( (set[setNr].pnt = (int *)realloc( (int *)set[setNr].pnt, set[setNr].anz_p*sizeof(int))) == NULL )
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); return(-1); }
      else
      {
        if(printFlag) printf(" set[%d]:%s reallocated and pnt %s added\n"
            , setNr, set[setNr].name, point[number].name );
      }
      set[setNr].pnt[ (set[setNr].anz_p-1) ] = number;
    }
    else
      set[setNr].anz_p=iinsert(&set[setNr].pnt, set[setNr].anz_p, number);

    if (point[number].nn>0)
    {
      for (k=0; k<point[number].nn; k++)
      {
        set[setNr].anz_n=iinsert(&set[setNr].node, set[setNr].anz_n, point[number].nod[k] );
      }
    }
  }
  else if ( type[0] == 'l' )
  {
    if (number<0)
    { if(printFlag) printf(" lineNr:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      /*
      for (n=0; n<set[setNr].anz_l; n++)
      {
        if( set[setNr].line[n] == number )
        { if(printFlag) printf(" ERROR in seta: line already stored in set %s\n", set[setNr].name); return(-1); }
      }
      */
      set[setNr].anz_l++;
      if( (set[setNr].line = (int *)realloc( (int *)set[setNr].line, set[setNr].anz_l*sizeof(int))) == NULL )
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); return(-1); }
      else
      {
        if(printFlag) printf(" set[%d]:%s reallocated and line %s added\n"
            , setNr, set[setNr].name, line[number].name );
      }
      set[setNr].line[ (set[setNr].anz_l-1) ] = number;
    }
    else
      set[setNr].anz_l= iinsert(&set[setNr].line, set[setNr].anz_l, number);
    
    if( line[number].nn >0 )
    {
      for (k=0; k<line[number].nn; k++)
      {
        set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n, line[number].nod[k] );
      }
    }
    if (line[number].ne>0)
    {
      for (k=0; k<line[number].ne; k++)
      {
        set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e, line[number].elem[k] );
      }
    }
  }
  else if ( type[0] == 'c' )
  {
    if (number<0)
    { if(printFlag) printf(" lcmbNr:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_c= iinsert(&set[setNr].lcmb, set[setNr].anz_c, number);
  }
  else if ( type[0] == 's' )
  {
    if (number<0)
    { if(printFlag) printf(" surfNr:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_s= iinsert(&set[setNr].surf, set[setNr].anz_s, number);
    if (surf[number].nn>0)
    {
      for (k=0; k<surf[number].nn; k++)
      {
        set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n, surf[number].nod[k] );
      }
    }
    if (surf[number].ne>0)
    {
      for (k=0; k<surf[number].ne; k++)
      {
        set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e, surf[number].elem[k] );
      }
    }
  }
  else if ( type[0] == 'b' )
  { 
    if (number<0)
    { if(printFlag) printf(" bodyNr:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_b= iinsert(&set[setNr].body, set[setNr].anz_b, number);

    if (body[number].nn>0)
    {
      for (k=0; k<body[number].nn; k++)
      {
        set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n,body[number].nod[k] );
      }
    }
    if (body[number].ne>0)
    {
      for (k=0; k<body[number].ne; k++)
      {
        set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e,body[number].elem[k] );
      }
    }
  }
  else if ( type[0] == 'L' )
  {
    if (number<0)
    { if(printFlag) printf(" NurlNr:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_nurl= iinsert(&set[setNr].nurl, set[setNr].anz_nurl, number);
  }
  else if ( type[0] == 'S' )
  {
    if (number<0)
    { if(printFlag) printf(" NursNr:%d does not exist\n", number ); return(-1); }
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    set[setNr].anz_nurs= iinsert(&set[setNr].nurs, set[setNr].anz_nurs, number);
    number=getShapeNr(nurbs[number].name);
    if(number>-1)  set[setNr].anz_sh= iinsert(&set[setNr].shp, set[setNr].anz_sh, number);
  }
  else if ( type[0] == 'j' )
  {
    if(set[setNr].type==1) /* seqa */
    {
      printf(" ERROR in seta: seqa not implemented for this type of entity\n");
      return(-1);
    }
    if((set[setNr].elf= (Elfaces *)realloc(set[setNr].elf, (set[setNr].anz_elf+1)*sizeof(Elfaces))) == NULL )
    { printf("ERROR: realloc failed in seta()\n\n" ); return(-1); }
    if(number)
    {
      set[setNr].elf[set[setNr].anz_elf].n=number;
      if((set[setNr].elf[set[setNr].anz_elf].v= (float *)calloc(number, sizeof(float))) == NULL )
      { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    }
    else set[setNr].elf[set[setNr].anz_elf].n=0;
    set[setNr].anz_elf++;
    return(set[setNr].anz_elf-1);
  }
  else
  {
    errMsg ("WARNING: in seta type:%s not recognized\n", type);
    return(-1);
  }
  return(1);
}


int sete( int setNr, char *type, char *mode)
{
  int i=0,s;
  int failFlag=0;

  /* check if all entities of setNr are also members in another sets */
  if ((type[0]=='s')&&(type[1]=='e'))
  {
    for(s=0; s<anz->sets; s++)
    {
      failFlag=0;
      if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
      {
        if(set[setNr].anz_n> set[s].anz_n) continue;
        if(set[setNr].anz_e> set[s].anz_e) continue;
        if(set[setNr].anz_f> set[s].anz_f) continue;
        if(set[setNr].anz_p> set[s].anz_p) continue;
        if(set[setNr].anz_l> set[s].anz_l) continue;
        if(set[setNr].anz_c> set[s].anz_c) continue;
        if(set[setNr].anz_s> set[s].anz_s) continue;
        if(set[setNr].anz_b> set[s].anz_b) continue;
        if(set[setNr].anz_se> set[s].anz_se) continue;
        if(set[setNr].anz_sh> set[s].anz_sh) continue;
        if(set[setNr].anz_nurl> set[s].anz_nurl) continue;
        if(set[setNr].anz_nurs> set[s].anz_nurs) continue;
        if((s!=setNr)&&(set[s].name!=NULL))
	{
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(0>ifind(&set[s].node, set[s].anz_n, set[setNr].node[i] )) break;
          }
          if(i!=set[setNr].anz_n) failFlag=1;
          for (i=0; i<set[setNr].anz_e; i++)
          {
            if(0>ifind(&set[s].elem, set[s].anz_e, set[setNr].elem[i] )) break;
          }
          if(i!=set[setNr].anz_e) failFlag=1;
          for (i=0; i<set[setNr].anz_f; i++)
          {
            if(0>ifind(&set[s].face, set[s].anz_f, set[setNr].face[i] )) break;
          }
          if(i!=set[setNr].anz_f) failFlag=1;
          for (i=0; i<set[setNr].anz_p; i++)
          {
            if(0>ifind(&set[s].pnt, set[s].anz_p, set[setNr].pnt[i] )) break;
          }
          if(i!=set[setNr].anz_p) failFlag=1;
          for (i=0; i<set[setNr].anz_l; i++)
          {
            if(0>ifind(&set[s].line, set[s].anz_l, set[setNr].line[i] )) break;
          }
          if(i!=set[setNr].anz_l) failFlag=1;
          for (i=0; i<set[setNr].anz_c; i++)
          {
            if(0>ifind(&set[s].lcmb, set[s].anz_c, set[setNr].lcmb[i] )) break;
          }
          if(i!=set[setNr].anz_c) failFlag=1;
          for (i=0; i<set[setNr].anz_s; i++)
          {
            if(0>ifind(&set[s].surf, set[s].anz_s, set[setNr].surf[i] )) break;
          }
          if(i!=set[setNr].anz_s) failFlag=1;
          for (i=0; i<set[setNr].anz_b; i++)
          {
            if(0>ifind(&set[s].body, set[s].anz_b, set[setNr].body[i] )) break;
          }
          if(i!=set[setNr].anz_b) failFlag=1;
          for (i=0; i<set[setNr].anz_se; i++)
          {
            if(0>ifind(&set[s].set, set[s].anz_se, set[setNr].set[i] )) break;
          }
          if(i!=set[setNr].anz_se) failFlag=1;
          for (i=0; i<set[setNr].anz_sh; i++)
          {
            if(0>ifind(&set[s].shp, set[s].anz_sh, set[setNr].shp[i] )) break;
          }
          if(i!=set[setNr].anz_sh ) failFlag=1;
          for (i=0; i<set[setNr].anz_nurl; i++)
          {
            if(0>ifind(&set[s].nurl, set[s].anz_nurl, set[setNr].nurl[i] )) break;
          }
          if(i!=set[setNr].anz_nurl) failFlag=1;
          for (i=0; i<set[setNr].anz_nurs; i++)
          {
            if(0>ifind(&set[s].nurs, set[s].anz_nurs, set[setNr].nurs[i] )) break;
          }
          if(i!=set[setNr].anz_nurs) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All entities from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(
                (set[setNr].anz_n==set[s].anz_n)&&
                (set[setNr].anz_e==set[s].anz_e)&&
                (set[setNr].anz_f==set[s].anz_f)&&
                (set[setNr].anz_p==set[s].anz_p)&&
                (set[setNr].anz_l==set[s].anz_l)&&
                (set[setNr].anz_c==set[s].anz_c)&&
                (set[setNr].anz_s==set[s].anz_s)&&
                (set[setNr].anz_b==set[s].anz_b)&&
                (set[setNr].anz_se==set[s].anz_se)&&
                (set[setNr].anz_sh==set[s].anz_sh)&&
                (set[setNr].anz_nurl==set[s].anz_nurl)&&
                (set[setNr].anz_nurs==set[s].anz_nurs) )
              { printf(" set:%s and %s share identical entities\n", set[setNr].name, set[s].name);
                  strcpy(parameter[0], set[s].name);
                  write2stack(1, parameter);
	      }
	    }
	  }
	}
      }
      else /* max */
      {
        if(set[setNr].anz_n< set[s].anz_n) continue;
        if(set[setNr].anz_e< set[s].anz_e) continue;
        if(set[setNr].anz_f< set[s].anz_f) continue;
        if(set[setNr].anz_p< set[s].anz_p) continue;
        if(set[setNr].anz_l< set[s].anz_l) continue;
        if(set[setNr].anz_c< set[s].anz_c) continue;
        if(set[setNr].anz_s< set[s].anz_s) continue;
        if(set[setNr].anz_b< set[s].anz_b) continue;
        if(set[setNr].anz_se< set[s].anz_se) continue;
        if(set[setNr].anz_sh< set[s].anz_sh) continue;
        if(set[setNr].anz_nurl< set[s].anz_nurl) continue;
        if(set[setNr].anz_nurs< set[s].anz_nurs) continue;
        if((s!=setNr)&&(set[s].name!=NULL))
	{
          for (i=0; i<set[s].anz_n; i++)
          {
            if(0>ifind(&set[setNr].node, set[setNr].anz_n, set[s].node[i] )) break;
          }
          if(i!=set[setNr].anz_n) failFlag=1;
          for (i=0; i<set[s].anz_e; i++)
          {
            if(0>ifind(&set[setNr].elem, set[setNr].anz_e, set[s].elem[i] )) break;
          }
          if(i!=set[s].anz_e) failFlag=1;
          for (i=0; i<set[s].anz_f; i++)
          {
            if(0>ifind(&set[setNr].face, set[setNr].anz_f, set[s].face[i] )) break;
          }
          if(i!=set[s].anz_f) failFlag=1;
          for (i=0; i<set[s].anz_p; i++)
          {
            if(0>ifind(&set[setNr].pnt, set[setNr].anz_p, set[s].pnt[i] )) break;
          }
          if(i!=set[s].anz_p) failFlag=1;
          for (i=0; i<set[s].anz_l; i++)
          {
            if(0>ifind(&set[setNr].line, set[setNr].anz_l, set[s].line[i] )) break;
          }
          if(i!=set[s].anz_l) failFlag=1;
          for (i=0; i<set[s].anz_c; i++)
          {
            if(0>ifind(&set[setNr].lcmb, set[setNr].anz_c, set[s].lcmb[i] )) break;
          }
          if(i!=set[s].anz_c) failFlag=1;
          for (i=0; i<set[s].anz_s; i++)
          {
            if(0>ifind(&set[setNr].surf, set[setNr].anz_s, set[s].surf[i] )) break;
          }
          if(i!=set[s].anz_s) failFlag=1;
          for (i=0; i<set[s].anz_b; i++)
          {
            if(0>ifind(&set[setNr].body, set[setNr].anz_b, set[s].body[i] )) break;
          }
          if(i!=set[s].anz_b) failFlag=1;
          for (i=0; i<set[s].anz_se; i++)
          {
            if(0>ifind(&set[setNr].set, set[setNr].anz_se, set[s].set[i] )) break;
          }
          if(i!=set[s].anz_se) failFlag=1;
          for (i=0; i<set[s].anz_sh; i++)
          {
            if(0>ifind(&set[setNr].shp, set[setNr].anz_sh, set[s].shp[i] )) break;
          }
          if(i!=set[s].anz_sh ) failFlag=1;
          for (i=0; i<set[s].anz_nurl; i++)
          {
            if(0>ifind(&set[setNr].nurl, set[setNr].anz_nurl, set[s].nurl[i] )) break;
          }
          if(i!=set[s].anz_nurl) failFlag=1;
          for (i=0; i<set[s].anz_nurs; i++)
          {
            if(0>ifind(&set[setNr].nurs, set[setNr].anz_nurs, set[s].nurs[i] )) break;
          }
          if(i!=set[s].anz_nurs) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All entities from set:%s are also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }
    }
    return(1);
  }

  for(s=0; s<anz->sets; s++)
  {
    if((s!=setNr)&&(set[s].name!=NULL))
    {
      if ((type[0]=='s')&&(type[1]=='h'))
      {
        if(!set[setNr].anz_sh) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_sh> set[s].anz_sh) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_sh; i++)
          {
            if(0>ifind(&set[s].shp, set[s].anz_sh, set[setNr].shp[i] )) break;
          }
          if(i!=set[setNr].anz_sh ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All shapes from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_n==set[s].anz_n) {  printf(" set:%s and %s share identical shapes\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_sh< set[s].anz_sh) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_sh; i++)
          {
            if(0>ifind(&set[setNr].shp, set[setNr].anz_sh, set[s].shp[i] )) break;
          }
          if(i!=set[s].anz_sh ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All shapes from set:%s are also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='n')
      {
        if(!set[setNr].anz_n) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_n> set[s].anz_n) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(0>ifind(&set[s].node, set[s].anz_n, set[setNr].node[i] )) break;
          }
          if(i!=set[setNr].anz_n ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All nodes from set:%s also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_n==set[s].anz_n) {  printf(" set:%s and %s share identical nodes\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_n< set[s].anz_n) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_n; i++)
          {
            if(0>ifind(&set[setNr].node, set[setNr].anz_n, set[s].node[i] )) break;
          }
          if(i!=set[s].anz_n ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All nodes from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='e')
      {
        if(!set[setNr].anz_e) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_e> set[s].anz_e) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_e; i++)
          {
            if(0>ifind(&set[s].elem, set[s].anz_e, set[setNr].elem[i] )) break;
          }
          if(i!=set[setNr].anz_e ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All elems from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more elems as setNr */
	    {
              if(set[setNr].anz_e==set[s].anz_e) {  printf(" set:%s and %s share identical elems\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_e< set[s].anz_e) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_e; i++)
          {
            if(0>ifind(&set[setNr].elem, set[setNr].anz_e, set[s].elem[i] )) break;
          }
          if(i!=set[s].anz_e ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All elems from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='p')
      {
        if(!set[setNr].anz_p) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_p> set[s].anz_p) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_p; i++)
          {
            if(0>ifind(&set[s].pnt , set[s].anz_p, set[setNr].pnt[i] )) break;
          }
          if(i!=set[setNr].anz_p ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All points from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_p==set[s].anz_p) {  printf(" set:%s and %s share identical points\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_p< set[s].anz_p) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_p; i++)
          {
            if(0>ifind(&set[setNr].pnt , set[setNr].anz_p, set[s].pnt[i] )) break;
          }
          if(i!=set[s].anz_p ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All points from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='c')
      {
        if(!set[setNr].anz_c) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_c> set[s].anz_c) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_c; i++)
          {
            if(0>ifind(&set[s].lcmb, set[s].anz_c, set[setNr].lcmb[i] )) break;
          }
          if(i!=set[setNr].anz_c ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All lcmbs from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_c==set[s].anz_c) {  printf(" set:%s and %s share identical lcmbs\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_c< set[s].anz_c) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_c; i++)
          {
            if(0>ifind(&set[setNr].lcmb, set[setNr].anz_c, set[s].lcmb[i] )) break;
          }
          if(i!=set[s].anz_c ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All lcmbs from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='l')
      {
        if(!set[setNr].anz_l) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_l> set[s].anz_l) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_l; i++)
          {
            if(0>ifind(&set[s].line, set[s].anz_l, set[setNr].line[i] )) break;
          }
          if(i!=set[setNr].anz_l ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All lines from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_l==set[s].anz_l) {  printf(" set:%s and %s share identical lines\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_l< set[s].anz_l) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_l; i++)
          {
            if(0>ifind(&set[setNr].line, set[setNr].anz_l, set[s].line[i] )) break;
          }
          if(i!=set[s].anz_l ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All lines from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='s')
      {
        if(!set[setNr].anz_s) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_s> set[s].anz_s) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_s; i++)
          {
            if(0>ifind(&set[s].surf, set[s].anz_s, set[setNr].surf[i] )) break;
          }
          if(i!=set[setNr].anz_s ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All surfs from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_s==set[s].anz_s) {  printf(" set:%s and %s share identical surfs\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_s< set[s].anz_s) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_s; i++)
          {
            if(0>ifind(&set[setNr].surf, set[setNr].anz_s, set[s].surf[i] )) break;
          }
          if(i!=set[s].anz_s ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All surfs from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='b')
      {
        if(!set[setNr].anz_b) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_b> set[s].anz_b) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_b; i++)
          {
            if(0>ifind(&set[s].body, set[s].anz_b, set[setNr].body[i] )) break;
          }
          if(i!=set[setNr].anz_b ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All bodies from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_b==set[s].anz_b) {  printf(" set:%s and %s share identical bodies\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_b< set[s].anz_b) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_b; i++)
          {
            if(0>ifind(&set[setNr].body, set[setNr].anz_b, set[s].body[i] )) break;
          }
          if(i!=set[s].anz_b ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All bodies from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='L')
      {
        if(!set[setNr].anz_nurl) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_nurl> set[s].anz_nurl) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_nurl; i++)
          {
            if(0>ifind(&set[s].nurl, set[s].anz_nurl, set[setNr].nurl[i] )) break;
          }
          if(i!=set[setNr].anz_nurl ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All nurls from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_nurl==set[s].anz_nurl) {  printf(" set:%s and %s share identical nurls\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_nurl< set[s].anz_nurl) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_nurl; i++)
          {
            if(0>ifind(&set[setNr].nurl, set[setNr].anz_nurl, set[s].nurl[i] )) break;
          }
          if(i!=set[s].anz_nurl ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All nurls from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

      else if (type[0]=='S')
      {
        if(!set[setNr].anz_nurs) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_nurs> set[s].anz_nurs) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_nurs; i++)
          {
            if(0>ifind(&set[s].nurs, set[s].anz_nurs, set[setNr].nurs[i] )) break;
          }
          if(i!=set[setNr].anz_nurs ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)
	    {
              printf(" All nurbs from set:%s are also in %s\n", set[setNr].name, set[s].name);
              strcpy(parameter[0], set[s].name);
              write2stack(1, parameter);
	    }
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_nurs==set[s].anz_nurs) {  printf(" set:%s and %s share identical nurbs\n", set[setNr].name, set[s].name);
                strcpy(parameter[0], set[s].name);
                write2stack(1, parameter);
	      }
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_nurs< set[s].anz_nurs) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_nurs; i++)
          {
            if(0>ifind(&set[setNr].nurs, set[setNr].anz_nurs, set[s].nurs[i] )) break;
          }
          if(i!=set[s].anz_nurs ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All nurbs from set:%s also in %s\n", set[s].name, set[setNr].name);
            strcpy(parameter[0], set[setNr].name);
            write2stack(1, parameter);
	  }
	}
      }

    }
  }
  return(1);
} 


int seti( int setNr, char *type, int sets, char dat[MAX_PARAM_PER_RECORD][MAX_LINE_LENGTH])
{
  int i,j,s,setnr[100];

  if(sets>100) { printf(" ERROR, to much sets\n"); return(0); }
  j=0;
  for(s=0; s<sets; s++) { i=getSetNr( dat[s] ); if((i>-1)&&(set[i].name!=NULL)) setnr[j++]=i; }
  sets=j;
  

  /* check if the entity is a member in all sets */
  if ((type[0]=='s')&&(type[1]=='e'))
  {
    for (j=0; j<set[setnr[0]].anz_n; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].node, set[setnr[s]].anz_n, set[setnr[0]].node[j] )) break;
      }
      if(s==sets) seta( setNr, "n", set[setnr[0]].node[j] );
    }
    for (j=0; j<set[setnr[0]].anz_e; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].elem, set[setnr[s]].anz_e, set[setnr[0]].elem[j] )) break;
      }
      if(s==sets) seta( setNr, "e", set[setnr[0]].elem[j] );
    }
    for (j=0; j<set[setnr[0]].anz_f; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].face, set[setnr[s]].anz_f, set[setnr[0]].face[j] )) break;
      }
      if(s==sets) seta( setNr, "f", set[setnr[0]].face[j] );
    }
    for (j=0; j<set[setnr[0]].anz_p; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].pnt, set[setnr[s]].anz_p, set[setnr[0]].pnt[j] )) break;
      }
      if(s==sets) seta( setNr, "p", set[setnr[0]].pnt[j] );
    }
    for (j=0; j<set[setnr[0]].anz_l; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].line, set[setnr[s]].anz_l, set[setnr[0]].line[j] )) break;
      }
      if(s==sets) seta( setNr, "l", set[setnr[0]].line[j] );
    }
    for (j=0; j<set[setnr[0]].anz_c; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].lcmb, set[setnr[s]].anz_c, set[setnr[0]].lcmb[j] )) break;
      }
      if(s==sets) seta( setNr, "c", set[setnr[0]].lcmb[j] );
    }
    for (j=0; j<set[setnr[0]].anz_s; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].surf, set[setnr[s]].anz_s, set[setnr[0]].surf[j] )) break;
      }
      if(s==sets) seta( setNr, "s", set[setnr[0]].surf[j] );
    }
    for (j=0; j<set[setnr[0]].anz_b; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].body, set[setnr[s]].anz_b, set[setnr[0]].body[j] )) break;
      }
      if(s==sets) seta( setNr, "b", set[setnr[0]].body[j] );
    }
    for (j=0; j<set[setnr[0]].anz_se; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].set, set[setnr[s]].anz_se, set[setnr[0]].set[j] )) break;
      }
      if(s==sets) seta( setNr, "se", set[setnr[0]].set[j] );
    }
    for (j=0; j<set[setnr[0]].anz_sh; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].shp, set[setnr[s]].anz_sh, set[setnr[0]].shp[j] )) break;
      }
      if(s==sets) seta( setNr, "sh", set[setnr[0]].shp[j] );
    }
    for (j=0; j<set[setnr[0]].anz_nurl; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].nurl, set[setnr[s]].anz_nurl, set[setnr[0]].nurl[j] )) break;
      }
      if(s==sets) seta( setNr, "L", set[setnr[0]].nurl[j] );
    }
    for (j=0; j<set[setnr[0]].anz_nurs; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].nurs, set[setnr[s]].anz_nurs, set[setnr[0]].nurs[j] )) break;
      }
      if(s==sets) seta( setNr, "S", set[setnr[0]].nurs[j] );
    }
  }
  else if ((type[0]=='s')&&(type[1]=='h'))
  {
    for (j=0; j<set[setnr[0]].anz_sh; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].shp, set[setnr[s]].anz_sh, set[setnr[0]].shp[j] )) break;
      }
      if(s==sets) seta( setNr, "sh", set[setnr[0]].shp[j] );
    }
  }
  else if (type[0]=='n')
  {
    for (j=0; j<set[setnr[0]].anz_n; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].node, set[setnr[s]].anz_n, set[setnr[0]].node[j] )) break;
      }
      if(s==sets) seta( setNr, "n", set[setnr[0]].node[j] );
    }
  }
  else if (type[0]=='e')
  {
    for (j=0; j<set[setnr[0]].anz_e; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].elem, set[setnr[s]].anz_e, set[setnr[0]].elem[j] )) break;
      }
      if(s==sets) seta( setNr, "e", set[setnr[0]].elem[j] );
    }
  }
  else if (type[0]=='f')
  {
    for (j=0; j<set[setnr[0]].anz_f; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].face, set[setnr[s]].anz_f, set[setnr[0]].face[j] )) break;
      }
      if(s==sets) seta( setNr, "f", set[setnr[0]].face[j] );
    }
  }
  else if (type[0]=='p')
  {
    for (j=0; j<set[setnr[0]].anz_p; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].pnt, set[setnr[s]].anz_p, set[setnr[0]].pnt[j] )) break;
      }
      if(s==sets) seta( setNr, "p", set[setnr[0]].pnt[j] );
    }
  }
  else if (type[0]=='l')
  {
    for (j=0; j<set[setnr[0]].anz_l; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].line, set[setnr[s]].anz_l, set[setnr[0]].line[j] )) break;
      }
      if(s==sets) seta( setNr, "l", set[setnr[0]].line[j] );
    }
  }
  else if (type[0]=='c')
  {
    for (j=0; j<set[setnr[0]].anz_c; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].lcmb, set[setnr[s]].anz_c, set[setnr[0]].lcmb[j] )) break;
      }
      if(s==sets) seta( setNr, "c", set[setnr[0]].lcmb[j] );
    }
  }
  else if (type[0]=='s')
  {
    for (j=0; j<set[setnr[0]].anz_s; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].surf, set[setnr[s]].anz_s, set[setnr[0]].surf[j] )) break;
      }
      if(s==sets) seta( setNr, "s", set[setnr[0]].surf[j] );
    }
  }
  else if (type[0]=='b')
  {
    for (j=0; j<set[setnr[0]].anz_b; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].body, set[setnr[s]].anz_b, set[setnr[0]].body[j] )) break;
      }
      if(s==sets) seta( setNr, "b", set[setnr[0]].body[j] );
    }
  }
  else if (type[0]=='L')
  {
    for (j=0; j<set[setnr[0]].anz_nurl; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].nurl, set[setnr[s]].anz_nurl, set[setnr[0]].nurl[j] )) break;
      }
      if(s==sets) seta( setNr, "L", set[setnr[0]].nurl[j] );
    }
  }
  else if (type[0]=='S')
  {
    for (j=0; j<set[setnr[0]].anz_nurs; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].nurs, set[setnr[s]].anz_nurs, set[setnr[0]].nurs[j] )) break;
      }
      if(s==sets) seta( setNr, "S", set[setnr[0]].nurs[j] );
    }
  }
  return(1);
} 



int embodies( int set1, int set2, double cof[4])
{
  int i,j,e,n;
  int etet, tets=0, extrapolflag=-1, includedFlag=-1;
  
  static Elements  *elem=NULL;   
  static Nodes     *nod=NULL;
  static Tetraeder *tet=NULL;
  static Rsort *rsort=NULL;

  static double *orig_ex=NULL, *orig_ey=NULL, *orig_ez=NULL, *sort_ex=NULL, *sort_ey=NULL, *sort_ez=NULL;
  static int *sort_enx=NULL, *sort_eny=NULL, *sort_enz=NULL, near_node[N_CLOSEST_TETS];
  int     n_closest_tets;


  /* split elements into tet's (with cg and volu) */

  if ( (nod = (Nodes *)realloc( (Nodes *)nod, (anz->nmax+1) * sizeof(Nodes))) == NULL )
  {
    printf("WARNING: realloc error embodies\n");
  }
  if((elem = (Elements *)realloc( (Elements *)elem, (set[set2].anz_e) * sizeof(Elements))) == NULL )
  {
    printf("WARNING: realloc error embodies\n");
  }
  for (j=1; j<=anz->nmax; j++)
  {
    nod[j].nx=node[j].nx;
    nod[j].ny=node[j].ny;
    nod[j].nz=node[j].nz;
  }
  for (i=0; i<set[set2].anz_e; i++)
  {
    e=set[set2].elem[i];
    if(e_enqire[e].type==4) elem[i].type=1;
    else if(e_enqire[e].type==6) elem[i].type=3;
    else elem[i].type=e_enqire[e].type;
    elem[i].nr=e;
    for (j=0; j<26; j++) elem[i].nod[j]=e_enqire[e].nod[j];
  }

  tets = splitElementsToTets(set[set2].anz_e, nod, elem, &tet);
  if(!tets) { printf(" No 3D master elements found\n"); return(set1); }

  /* delete unusable tets (small volume) */
  j=0;
  for(i=0; i<tets; i++)
  {
    if(tet[i].v<SMALL_TET_VOLUME) { if(printFlag) printf("Scip Tet:%d vol:%e < SMALL_TET_VOLUME\n", i, tet[i].v); continue; }
    if(j<i)
    {   
      for(n=0;n<4; n++) tet[j].n[n]=tet[i].n[n];
      for(n=0;n<3; n++) tet[j].cg[n]=tet[i].cg[n];
      tet[j].v=tet[i].v;
      tet[j].e=tet[i].e;
    }
    j++;
  }
  tets=j;
  if(printFlag) printf(" %d tets created\n", tets);


  if(set[set2].anz_n>tets)
  {
    if ( (rsort = (Rsort *)malloc( (set[set2].anz_n+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  }
  else
  {
    if ( (rsort = (Rsort *)malloc( (tets+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  }
  
  /* get the close tets */
  /* search the closest elements based on the closest cg */
  if ( (orig_ex = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_ey = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_ez = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ex = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ey = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ez = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_enx = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_eny = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_enz = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ex[i]=tet[i].cg[0];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ex[i]=rsort[i].r;
    sort_enx[i]=rsort[i].i;
  }
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ey[i]=tet[i].cg[1];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ey[i]=rsort[i].r;
    sort_eny[i]=rsort[i].i;
  }
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ez[i]=tet[i].cg[2];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ez[i]=rsort[i].r;
    sort_enz[i]=rsort[i].i;
  }

  if((int)N_CLOSEST_TETS/10<tets)  n_closest_tets= (int)N_CLOSEST_TETS/10; else n_closest_tets= tets;

  /* check if all nodes of setNr are inside the elements of elsetNr */
  for(i=0; i<set[set1].anz_n; i++)
  {
    /* search a close element */
    near3d(orig_ex,orig_ey,orig_ez,sort_ex,sort_ey,sort_ez,sort_enx,sort_eny,sort_enz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny, node[set[set1].node[i]].nz, tets, &near_node[0], n_closest_tets);

    etet=calcCoefficientsTet(set[set1].node[i], &near_node[0], n_closest_tets, node, tet, cof, extrapolflag);
    if(etet>-1)
    {
      includedFlag=set[set1].node[i];
      break;
    }
    //for (j=0; j<n_closest_tets; j++) printf("node:%d nearest tets:%d e:%d i:%d\n",set[set1].node[i], near_node[j]+1, etet, includedFlag);
  }
  if(orig_ex) free(orig_ex);
  if(orig_ey) free(orig_ey);
  if(orig_ez) free(orig_ez);
  if(sort_ex) free(sort_ex);
  if(sort_ey) free(sort_ey);
  if(sort_ez) free(sort_ez);
  if(sort_enx) free(sort_enx);
  if(sort_eny) free(sort_eny);
  if(sort_enz) free(sort_enz);
  free(nod);
  free(elem);
  free(tet);
  nod=NULL;
  elem=NULL;

  printf("done\n");  
  return(includedFlag);
}


      
int setr( int setNr, char *type, int number)
{
  int i,j;

  if( set[setNr].lock == 1 ) return(-1);
  if( set[setNr].name == (char *)NULL )
  {
    errMsg(" ERROR: setNr:%d is undefined\n", setNr);
    return(-1);
  }
  if( number<0)
  {
    errMsg(" ERROR in setr: A negative entity-index:%d was used\n", number);
    return(-1);
  }

  if (( type[0] == 'r' )||(( type[0] == 's' )&&( type[1] == 'e' )))
  {
    /* set remove  */
    i=iremove(&set[setNr].set, set[setNr].anz_se, number);
    if(i<set[setNr].anz_se)
    {
      set[setNr].anz_se=i;
    }
    else { if(printFlag) printf(" set:%s not in set:%s\n", set[number].name, set[setNr].name); }
  }
  else if (type[0]=='v')
  {
    /* value remove  */
    i=iremove(&set[setNr].valu, set[setNr].anz_v, number);
    if(i<set[setNr].anz_v)
    {
      set[setNr].anz_v=i;
    }
    else { if(printFlag) printf(" value:%s not in set:%s\n", value[number].name, set[setNr].name); }
  }
  else if (( type[0] == 'h' )||(( type[0] == 's' )&&( type[1] == 'h' )))
  {
    /* set remove  */
    i=iremove(&set[setNr].shp, set[setNr].anz_sh, number);
    if(i<set[setNr].anz_sh)
    {
      set[setNr].anz_sh=i;
    }
    else { if(printFlag) printf(" shape:%s not in set:%s\n", shape[number].name, set[setNr].name); }
  }
  else if (type[0]=='n')
  {
    /* node remove  */
    i=iremove(&set[setNr].node, set[setNr].anz_n, number);
    if(i<set[setNr].anz_n)
    {
      set[setNr].anz_n=i;
    }
    else { if(printFlag) printf(" node:%d not in set:%s\n", number, set[setNr].name); }
  }
  else if (type[0]=='e')
  {
    /* element remove  */
    i=iremove(&set[setNr].elem, set[setNr].anz_e, number);
    if(i<set[setNr].anz_e)
    {
      set[setNr].anz_e=i;
    }
    else { if(printFlag) printf(" element:%d not in set:%s\n", number, set[setNr].name); }
  }
  else if (type[0]=='f')
  {
    /* element remove  */
    i=iremove(&set[setNr].face, set[setNr].anz_f, number);
    if(i<set[setNr].anz_f)
    {
      set[setNr].anz_f=i;
    }
    else { if(printFlag) printf(" face:%d not in set:%s\n", number, set[setNr].name); }
  }
  else if (type[0]=='p')
  {
    /* point remove  */
    i=iremove(&set[setNr].pnt, set[setNr].anz_p, number);
    if(i<set[setNr].anz_p)
    {
      set[setNr].anz_p=i;

      /* remove also embedded nodes */
      for (j=0; j<point[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, point[number].nod[j]);
      }        
    }
    else { if(printFlag) printf(" pnt:%s not in set:%s\n", point[number].name, set[setNr].name); }
  }
  else if (type[0]=='l')
  {
    /* line remove  */
    i=iremove(&set[setNr].line, set[setNr].anz_l, number);
    if(i<set[setNr].anz_l)
    {
      set[setNr].anz_l=i;

      /* remove also embedded nodes */
      for (j=0; j<line[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, line[number].nod[j]);
      }        

      /* remove also embedded elements */
      for (j=0; j<line[number].ne; j++)
      {
        set[setNr].anz_e=iremove(&set[setNr].elem, set[setNr].anz_e, line[number].elem[j]);
      }        
    }
    else { if(printFlag) printf(" line:%s not in set:%s\n", line[number].name, set[setNr].name); }
  }
  else if (type[0]=='c')
  {
    /* lcmb remove  */
    i=iremove(&set[setNr].lcmb, set[setNr].anz_c, number);
    if(i<set[setNr].anz_c)
    {
      set[setNr].anz_c=i;
    }
    else{  if(printFlag) printf(" lcmb:%s not in set:%s\n", lcmb[number].name, set[setNr].name); }
  }
  else if (type[0]=='s')
  {
    /* surface remove  */
    i=iremove(&set[setNr].surf, set[setNr].anz_s, number);
    if(i<set[setNr].anz_s)
    {
      set[setNr].anz_s=i;

      /* remove also embedded nodes */
      for (j=0; j<surf[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, surf[number].nod[j]);
      }        

      /* remove also embedded elements */
      for (j=0; j<surf[number].ne; j++)
      {
        set[setNr].anz_e=iremove(&set[setNr].elem, set[setNr].anz_e, surf[number].elem[j]);
      }        
    }
    else{  if(printFlag) printf(" surf:%s not in set:%s\n", surf[number].name, set[setNr].name); }
  }
  else if (type[0]=='b')
  {
    /* body remove  */
    i=iremove(&set[setNr].body, set[setNr].anz_b, number);
    if(i<set[setNr].anz_b)
    {
      set[setNr].anz_b=i;

      /* remove also embedded nodes */
      for (j=0; j<body[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, body[number].nod[j]);
      }        

      /* remove also embedded elements */
      for (j=0; j<body[number].ne; j++)
      {
        set[setNr].anz_e=iremove(&set[setNr].elem, set[setNr].anz_e, body[number].elem[j]);
      }        
    }
    else{ if(printFlag) printf(" body:%s not in set:%s\n", body[number].name, set[setNr].name); }
  }
  else if (type[0]=='L')
  {
    /* nurl remove  */
    i=iremove(&set[setNr].nurl, set[setNr].anz_nurl, number);
    if(i<set[setNr].anz_nurl)
    {
      set[setNr].anz_nurl=i;
    }
    else{ if(printFlag) printf(" nurl:%s not in set:%s\n", nurbl[number].name, set[setNr].name); }
  }
  else if (type[0]=='S')
  {
    /* nurs remove  */
    i=iremove(&set[setNr].nurs, set[setNr].anz_nurs, number);
    if(i<set[setNr].anz_nurs)
    {
      set[setNr].anz_nurs=i;
    }
    else{ if(printFlag) printf(" nurs:%s not in set:%s\n", nurbs[number].name, set[setNr].name); }
  }
  else if ( type[0] == 'j' )
  {
    if(number<set[setNr].anz_elf-1)
    {
      for ( i=number; i<set[setNr].anz_elf-1; i++)
      {
        set[setNr].elf[number].e=set[setNr].elf[number+1].e;
        set[setNr].elf[number].f=set[setNr].elf[number+1].f;
      }
      if((set[setNr].elf= (Elfaces *)realloc(set[setNr].elf, (set[setNr].anz_elf)*sizeof(Elfaces))) == NULL )
      { printf("ERROR: realloc failed in seta()\n\n" ); return(-1); }
      set[setNr].anz_elf--;
    }
    else if(number==set[setNr].anz_elf-1)
    {
      if((set[setNr].elf= (Elfaces *)realloc(set[setNr].elf, (set[setNr].anz_elf)*sizeof(Elfaces))) == NULL )
      { printf("ERROR: realloc failed in seta()\n\n" ); return(-1); }
      set[setNr].anz_elf--;
    }
    else{ if(printFlag) printf(" elf:%d not in set:%s\n", number, set[setNr].name); }
  }
  else
  {
    errMsg ("WARNING: in setr type:%s not recognized\n", type );
    return(-1);
  }
  return(1);
}


/*  seqr works only for sequences (set[].type==1) with points and nodes */
int seqr( int setNr, char *type, int number)
{
  int i, n;
  int *puf=NULL;
  int flag=0;

  if( set[setNr].name == (char *)NULL )
  {
    errMsg(" ERROR: setNr:%d is undefined\n", setNr);
    return(0);
  }
  if( number<0)
  {
    errMsg(" ERROR in seqr: A negative entity-index:%d was used\n", number);
    return(-1);
  }

  if ( set[setNr].type!=1)
  {
    errMsg(" FATAL ERROR: in seqr, set:%s is not a sequence\n", set[setNr].name );
  }
  else if (type[0]=='n')
  {
    /* puffer allocieren  */
    if ( (puf = (int *)malloc( (set[setNr].anz_n+1)*sizeof(int))) == NULL )
      printf(" ERROR: malloc failed in setr\n\n");

    n=0;
    for ( i=0; i<set[setNr].anz_n; i++)
    {
      if (set[setNr].node[i] != number)
      {
        puf[n] = set[setNr].node[i];
        n++;
      }
      else
        flag=1;
    }

    if (flag)
    {
      set[setNr].anz_n--;
      /* if the set is empty, delete the set */
      if((set[setNr].anz_p <1)&& (set[setNr].anz_n <1)) 
      {
        delSet(set[setNr].name); free(puf); return(0);
      }
      if ((set[setNr].node = (int *)realloc( (int *)set[setNr].node,(set[setNr].anz_n+1)*sizeof(int)) ) == NULL)
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); free(puf); return(0); }
      else if(printFlag)
        printf (" set[%d]:%s reallocated and node %d removed\n", setNr, set[setNr].name, number );

      for ( i=0; i<set[setNr].anz_n; i++)
      {
        set[setNr].node[i] = puf[i];
      }
    }
    else if(printFlag) printf(" node:%d not in set:%s\n", number, set[setNr].name);
    free(puf);
  }
  else if (type[0]=='p')
  {
    /* puffer allocieren  */
    if ( (puf = (int *)malloc( (set[setNr].anz_p+1)*sizeof(int))) == NULL )
      printf(" ERROR: malloc failed in setr\n\n");

    n=0;
    for ( i=0; i<set[setNr].anz_p; i++)
    {
      if (set[setNr].pnt[i] != number)
      {
        puf[n] = set[setNr].pnt[i];
        n++;
      }
      else
        flag=1;
    }

    if (flag)
    {
      set[setNr].anz_p--;
      /* if the set is empty, delete the set */
      if((set[setNr].anz_p <1)&& (set[setNr].anz_n <1)) 
      {
        /* suche abhaengige linien, redefine if necessary */
        for (i=0; i<anzGeo->l; i++)
        {
          if(( line[i].typ == 's' )&&( setNr == line[i].trk ))
          {
            line[i].typ = ' ';
          }
        }
        delSet(set[setNr].name); free(puf); return(0);
      }
      if ((set[setNr].pnt = (int *)realloc( (int *)set[setNr].pnt,(set[setNr].anz_p+1)*sizeof(int)) ) == NULL)
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); free(puf); return(0); }
      else if(printFlag)
        printf (" set[%d]:%s reallocated and pnt %s removed\n", setNr, set[setNr].name, point[number].name );

      for ( i=0; i<set[setNr].anz_p; i++)
      {
        set[setNr].pnt[i] = puf[i];
      }
    }
    else if(printFlag) printf(" pnt:%s not in set:%s\n", point[number].name, set[setNr].name);
    free(puf);
  }
  return(1);
}


double calcLineLength(int l)
{
  int n;
  double p1[3], p2[3], p1p2[3], lp1p2, lmax=0.;

  repLine(l);
  p1[0]=line[l].ip[0]*scale->w+scale->x;
  p1[1]=line[l].ip[1]*scale->w+scale->y;
  p1[2]=line[l].ip[2]*scale->w+scale->z;
  for (n=3; n<line[l].nip; n+=3)
  {
    p2[0]=line[l].ip[n]*scale->w+scale->x;
    p2[1]=line[l].ip[n+1]*scale->w+scale->y;
    p2[2]=line[l].ip[n+2]*scale->w+scale->z;
    v_result( p1, p2, p1p2 );
    lp1p2=v_betrag( p1p2 );
    lmax+=lp1p2;

    p1[0]=p2[0];
    p1[1]=p2[1];
    p1[2]=p2[2];
  }
  return(lmax);
}


/* returns 0 if commands are to be skipped */
int pre_while(char *type, char **ptr_string, int *na, int *nb, FILE *handle1, int *addFlag, int *gtolFlag, int wFlag, int ifFlag)
{
  typedef struct{
    int flag;              // whileFlag: 
                           // 0 do not restore and do not not execute
                           // 1 execute command
                           // 2 store and execute command
                           // 3 restore and execute in commandoInterpreter,
                           // 4 store command but not execute
                           // 5 restore command but not execute
    int clines;            // counts command lines
    int lineptr;           // points to the active command line in the stack (this parameters of that line will be returned)
    int prev_while;        // previous while loop (previous stack)
    int *next_while;       // embedded whiles, all while-command lines in the active loop have to be addressed.  
    int whiles;            // counts embedded while loops
    int whileptr;          // points to the next embedded while loop
    char arg1[MAX_LINE_LENGTH];
    char arg2[MAX_LINE_LENGTH];
    char operator[MAX_LINE_LENGTH];
    char **type;           // command name
    char **string;         // complete command string 
    int *na;
    int *nb;
    FILE **handle1;
    int *addFlag;
    int *gtolFlag;
  }whileStack;

  static whileStack *stack=NULL;
  static int stack_ptr=-1;             // active while loop
  static int nwhiles=0;             // allocated loops
  int   i,j;
  int   new_length,flag;
  char  arg1[MAX_LINE_LENGTH], operator[MAX_LINE_LENGTH], arg2[MAX_LINE_LENGTH];

  //printf("hallo in pre_while: type:%s str:%s nab:%d %d addf:%d gtolf:%d ",type,*ptr_string,*na,*nb,*addFlag,*gtolFlag);
  //printf("hallo whileFlag:%d\n",wFlag);

  // store the command
  // wFlag==2,4: save the command (makes here no difference)
  if((wFlag==2)||(wFlag==4))
  {
    if ((stack[stack_ptr].type = (char **)realloc(stack[stack_ptr].type, (stack[stack_ptr].clines+1)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    if ((stack[stack_ptr].type[stack[stack_ptr].clines] = (char *)malloc( (strlen(type)+1)*sizeof(char)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    strcpy(stack[stack_ptr].type[stack[stack_ptr].clines], type);

    if ((stack[stack_ptr].string = (char **)realloc(stack[stack_ptr].string, (stack[stack_ptr].clines+1)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    if ((stack[stack_ptr].string[stack[stack_ptr].clines] = (char *)malloc( (strlen(*ptr_string)+2)*sizeof(char)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    strcpy(stack[stack_ptr].string[stack[stack_ptr].clines], *ptr_string);

    if ((stack[stack_ptr].na = (int *)realloc(stack[stack_ptr].na, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].na[stack[stack_ptr].clines]=*na;

    if ((stack[stack_ptr].nb = (int *)realloc(stack[stack_ptr].nb, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].nb[stack[stack_ptr].clines]=*nb;

    if ((stack[stack_ptr].handle1 = (FILE **)realloc(stack[stack_ptr].handle1, (stack[stack_ptr].clines+1)*sizeof(FILE *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].handle1[stack[stack_ptr].clines]=handle1;

    if ((stack[stack_ptr].addFlag = (int *)realloc(stack[stack_ptr].addFlag, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].addFlag[stack[stack_ptr].clines]=*addFlag;

    if ((stack[stack_ptr].gtolFlag = (int *)realloc(stack[stack_ptr].gtolFlag, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].gtolFlag[stack[stack_ptr].clines]=*gtolFlag;

    stack[stack_ptr].clines++;

    //printf("hallo stored command[%d]:%s \n",stack[stack_ptr].clines, *ptr_string);
    return(wFlag);// unmodified
  }

  // restore the next command
  else if((wFlag==3)||(wFlag==5))
  {
    if(stack[stack_ptr].lineptr<stack[stack_ptr].clines)
    {
      strcpy(type,stack[stack_ptr].type[stack[stack_ptr].lineptr]);
      new_length=strlen(stack[stack_ptr].string[stack[stack_ptr].lineptr])+1;
      if(new_length<MAX_LINE_LENGTH) new_length=MAX_LINE_LENGTH;
      if( (*ptr_string=realloc(*ptr_string, (new_length)*sizeof(char)))== NULL ) { printf(" ERROR: realloc failed in pre_while()\n"); return(0); } 
      *ptr_string[0]=0;
      strcpy(*ptr_string,stack[stack_ptr].string[stack[stack_ptr].lineptr]);
      *na=stack[stack_ptr].na[stack[stack_ptr].lineptr];
      *nb=stack[stack_ptr].nb[stack[stack_ptr].lineptr];
      handle1=stack[stack_ptr].handle1[stack[stack_ptr].lineptr];
      *addFlag=stack[stack_ptr].addFlag[stack[stack_ptr].lineptr];
      *gtolFlag=stack[stack_ptr].gtolFlag[stack[stack_ptr].lineptr];
      stack[stack_ptr].lineptr++;
    }
    else { printf("ERROR: in while, no more commands (%d from %d stack_ptr:%d\n", stack[stack_ptr].lineptr, stack[stack_ptr].clines, stack_ptr); exit(0); }

    //printf("hallo restored command[%d]:%s\n", stack[stack_ptr].lineptr, *ptr_string );
    return(wFlag); // unmodified
  }

  else if(compare( type,"WHILE",2)==2 )
  {
    //printf("hallo %s stackptr:%d\n",type,stack_ptr);

    sscanf(*ptr_string, "%*s %s %s %s", arg1, operator, arg2);

    i=getValuNr(arg1);
    if (i>-1) strcpy(arg1,value[i].string );
    i=getValuNr(arg2);
    if (i>-1) strcpy(arg2,value[i].string );

    if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=3; else flag=0; }
    else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=3; else flag=0; }
    else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=3; else flag=0; }
    else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=3; else flag=0; }
    else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=3; else flag=0; }
    else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=3; else flag=0; }
    else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 

    //printf(" args:%s %s\n",arg1,arg2);
    //printf(" nwhiles:%d stackptr:%d flag:%d wflag:%d\n", nwhiles, stack_ptr, flag, wFlag);
    if(flag==1) return(1);  // error or strings are not equal


    if((wFlag==-3)||(wFlag==-5)) // restore the next command
    {
      //printf(" an open while (stack_ptr:%d) references this while:%d in a sequencial order:%d\n",stack_ptr, stack[stack_ptr].next_while[stack[stack_ptr].whileptr], stack[stack_ptr].whileptr);
      stack_ptr=stack[stack_ptr].next_while[stack[stack_ptr].whileptr++];        // set the pointer to the next while
      //printf(" change stack_ptr to:%d\n", stack_ptr);
      if((flag==3)&& (wFlag==-3)) stack[stack_ptr].flag=3;  // restore and execute (all commands are stored and repeated)
      else stack[stack_ptr].flag=5;  //  restore and do not not execute
      return(stack[stack_ptr].flag);
    }
    else if((stack_ptr<0)||(wFlag==-2)||(wFlag==-4)) // create a new while-stack as long as the commands have to be saved
    {
      if ((stack = (whileStack *)realloc( (whileStack *)stack, (nwhiles+1)*sizeof(whileStack)) ) == NULL )
      { printf("\n\nERROR: realloc failure, whileStack\n\n"); return(1); }
      if((flag==3)&&(wFlag>=-2)) stack[nwhiles].flag=2;  // store and execute
      else stack[nwhiles].flag=4;  // store but not execute
      stack[nwhiles].whileptr=0;
      stack[nwhiles].clines=0;
      stack[nwhiles].prev_while=stack_ptr;
      stack[nwhiles].whiles=-1;
      stack[nwhiles].next_while=NULL;
      // an eventually open while has to reference this while in a sequencial order
      if(stack_ptr>-1)
      {
        stack[stack_ptr].whiles++;
        //printf(" an open while (stack_ptr:%d) will reference this while:%d in a sequencial order:%d\n",stack_ptr, nwhiles, stack[stack_ptr].whiles);
        if ((stack[stack_ptr].next_while = (int *)realloc(stack[stack_ptr].next_while , (stack[stack_ptr].whiles+1)*sizeof(int)) ) == NULL )
        { printf("\n\nERROR: realloc failure, whileStack\n\n"); return(1); }
        stack[stack_ptr].next_while[stack[stack_ptr].whiles]=nwhiles;
      }
      sscanf(*ptr_string, "%*s %s %s %s", arg1, operator, arg2);
      strcpy(stack[nwhiles].arg1,arg1);
      strcpy(stack[nwhiles].arg2,arg2);
      strcpy(stack[nwhiles].operator,operator);
      stack[nwhiles].type=NULL;
      stack[nwhiles].string=NULL;
      stack[nwhiles].na=NULL;
      stack[nwhiles].nb=NULL;
      stack[nwhiles].handle1=NULL;
      stack[nwhiles].addFlag=NULL;
      stack[nwhiles].gtolFlag=NULL;
      stack_ptr=nwhiles;
      //printf(" new stack_ptr:%d\n", stack_ptr);
      return(stack[nwhiles++].flag);
    }
    printf(" talk to the programmer! should NEVER be here.  stack_ptr:%d flag:%d\n", stack_ptr, flag); exit(0);
  }

  else if(compare( type,"ENDWHILE",3)==3 )
  {
    //printf("hallo %s stackptr:%d stack[stack_ptr].flag:%d ifFlag:%d\n",type,stack_ptr, stack[stack_ptr].flag, ifFlag);

    // if the while was skipped from the beginning continue executing
    if((stack[stack_ptr].flag == 0)||(stack[stack_ptr].flag == 4))
    {
      // for seriell whiles the stack_ptr has to be decreased by the amount of seriell whiles
      stack[stack_ptr].lineptr=0;
      stack_ptr = stack[stack_ptr].prev_while;
      //printf("  3 changed stack_ptr:%d\n", stack_ptr);
      if(stack_ptr>=0)
        return(stack[stack_ptr].flag);
      else return(1);
    }
    else
    {
      // reset the embedded while counter
      stack[stack_ptr].whileptr=0;
      stack[stack_ptr].lineptr=0;

      // restore the command line to check if the loop has finished
      strcpy(arg1,stack[stack_ptr].arg1);
      strcpy(arg2,stack[stack_ptr].arg2);
      strcpy(operator,stack[stack_ptr].operator);
	//printf(" 1args:%s %s  operator:%s\n",arg1,arg2,operator);

      i=getValuNr(arg1);
      if (i>-1) strcpy(arg1,value[i].string );
      i=getValuNr(arg2);
      if (i>-1) strcpy(arg2,value[i].string );
	//printf(" 2args:%s %s  operator:%s\n",arg1,arg2,operator);
  
      if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=3; else flag=1; }
      else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=3; else flag=1; }
      else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=3; else flag=1; }
      else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=3; else flag=1; }
      else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=3; else flag=1; }
      else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=3; else flag=1; }
      else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 
      stack[stack_ptr].flag=flag;

      //printf("  flag:%d\n", flag);

      // if finished
      if((flag==1)||(ifFlag==0))
      {
        stack_ptr = stack[stack_ptr].prev_while;
	//printf("  4 changed stack_ptr:%d new_flag:%d\n", stack_ptr, stack[stack_ptr].flag);

        // free if stack_ptr is <0 (first while is closed)
	if(stack_ptr<0)
	{
	  for(i=0; i<nwhiles; i++)
	  {
	    for(j=0; j<stack[i].clines; j++)
	    {
              free(stack[i].type[j]);
              free(stack[i].string[j]);
            }
            free(stack[i].next_while);
            free(stack[i].type);
            free(stack[i].string);
            free(stack[i].na);
            free(stack[i].nb);
            free(stack[i].handle1);
            free(stack[i].addFlag);
            free(stack[i].gtolFlag);
            stack[i].type=NULL;
            stack[i].string=NULL;
            stack[i].next_while=NULL;
            stack[i].na=NULL;
            stack[i].nb=NULL;
            stack[i].handle1=NULL;
            stack[i].addFlag=NULL;
            stack[i].gtolFlag=NULL;

	  }
          nwhiles=0;
	}
        if(stack_ptr>=0)
          return(stack[stack_ptr].flag);
        else return(1);
      }
      return(3);
    }
  }

  //printf("hallo return pre_while\n");
  return(0);
}


/* returns 0 if commands are to be skipped */
int pre_if(char *string)
{
  int i,args, flag;
  char type[MAX_LINE_LENGTH], arg1[MAX_LINE_LENGTH], operator[MAX_LINE_LENGTH], arg2[MAX_LINE_LENGTH];

  /* stack for the 'if' blocks */
  static int *ifstack=NULL;
  static int *ifdone=NULL;
  static int ifstack_ptr=0;


  //printf("IF string:%s\n",string);
  args=sscanf(string, "%s %s %s %s", type, arg1, operator, arg2);
  for(i=0;i<strlen(type); i++) type[i]=toupper(type[i]);

  if(compare( type,"IF",2)==2 )
  {
    if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=1; else flag=0; }
    else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=1; else flag=0; }
    else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=1; else flag=0; }
    else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=1; else flag=0; }
    else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=1; else flag=0; }
    else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=1; else flag=0; }
    else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 

    if ((ifstack = (int *)realloc( (int *)ifstack, (ifstack_ptr+1)*sizeof(int)) ) == NULL )
      { printf("\n\nERROR: realloc failure, ifstack ptr:%d\n\n", ifstack_ptr); return(1); }
    if ((ifdone = (int *)realloc( (int *)ifdone, (ifstack_ptr+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, ifdone ptr:%d\n\n", ifstack_ptr); return(1); }
    ifstack[ifstack_ptr]=flag;
    ifdone[ifstack_ptr]=flag;
    //printf(" IF stackptr:%d ifdone = flag:%d\n",ifstack_ptr,flag);
    // check if the block is active
    for(i=0; i<ifstack_ptr; i++) { if(!ifstack[i] ) { flag=0; break; }}
    //printf(" upper if states checked: flag:%d\n",flag);
    ifstack_ptr++;
    return(flag);
  }
  else if(compare( type,"ELSE",2)==2 )
  {
    if(ifstack_ptr>0)
    {
      if(args>1)
      {
        for(i=0;i<strlen(arg1); i++) arg1[i]=toupper(arg1[i]);
        if(compare( arg1,"IF",2)==2 )
	{
          sscanf(string, "%*s %*s %s %s %s", arg1, operator, arg2);
          if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=1; else flag=0; }
          else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=1; else flag=0; }
          else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=1; else flag=0; }
          else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=1; else flag=0; }
          else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=1; else flag=0; }
          else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=1; else flag=0; }
          else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 
          if(ifdone[ifstack_ptr-1]==1) flag=0;
          ifstack[ifstack_ptr-1]=flag;
          if(flag==1) ifdone[ifstack_ptr-1]=1;
          // check if the block is active
          for(i=0; i<ifstack_ptr-1; i++) { if(!ifstack[i] ) { flag=0; break; }}
          //printf(" upper if states checked: flag:%d\n",flag);
          return(flag);
	}
      }
      else
      {
         if(ifdone[ifstack_ptr-1]==1) flag=0; else flag=1;
        ifstack[ifstack_ptr-1]=flag;
        if(flag==1) ifdone[ifstack_ptr-1]=1;
        //printf(" ELSE stackptr:%d flag:%d ifdone:%d\n",ifstack_ptr-1, flag, ifdone[ifstack_ptr-1]);
        // check if the block is active
        for(i=0; i<ifstack_ptr-1; i++) { if(!ifstack[i] ) { flag=0; break; }}
        //printf(" upper if states checked: flag:%d\n",flag);
        return(flag);
      }
    }
    else return(1);
  }
  else if(compare( type,"ENDIF",2)==2 )
  {
    ifdone[ifstack_ptr-1]=flag=1;
    //printf(" ENDIF stackptr:%d flag:%d\n",ifstack_ptr-1, flag);
    // check if the block is active
    for(i=0; i<ifstack_ptr-1; i++) { if(!ifstack[i] ) { flag=0; break; }}
    //printf(" upper if states checked: flag:%d\n",flag);
    ifstack_ptr--;
    return(flag);
  }

  return(0);
}


double pre_length(char *setname)
{
  int   i,n;
  int   setNr,l;
  double L=0, Ll=0;
  //double sum_valLe=0., value=0.;

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return(-1);
  }

  for(i=0; i<set[setNr].anz_l; i++)
  {
    l=set[setNr].line[i];
    Ll=calcLineLength(l);
    L+=Ll;
  }
  printf("LENGTH:%e", L);
  //if(anz->l) printf(" AVERAGE-VALUE:%f\n", sum_valLe/vol);
  //else
    printf("\n");
  if(valuestackFlag)
  {
    n=1;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    sprintf(valuestack[valuestack_ptr++],"%e", L );
    printf(" written to stack\n");
  }

  return(L);
}


double pre_area(char *setname)
{
  int   i,j,n;
  int   nr, setNr;
  double A=0., Ae, Abuf, p[3][3], pcg[3], Ix, Iy, Ixy;
  double xcg=0., ycg=0., zcg=0.;
  double sum_valAe=0., value=0.;

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return(-1);
  }
  if (set[setNr].anz_f<1)
  {
    printf (" ERROR: set:%s does not contain faces\n", setname);
    return(-1);
  }

  for(i=0; i<set[setNr].anz_f; i++)
  {
    Abuf=A;
    nr=set[setNr].face[i];
    switch(face[nr].type)
    {
      case 7:
      for(j=0; j<3; j++)
      {
        p[j][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
        p[j][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
        p[j][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      }
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      case 8:
      p[0][0]=node[face[nr].nod[0]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[0]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[0]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[1]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[1]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[1]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[2]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[2]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[2]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      case 9:
      for(j=0; j<3; j++)
      {
        p[j][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
        p[j][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
        p[j][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      }
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      n=j=0;
      p[n][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
      p[n][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
      p[n++][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      for(j=2; j<4; j++)
      {
        p[n][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
        p[n][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
        p[n++][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      }
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      case 10:
      p[0][0]=node[face[nr].nod[0]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[0]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[0]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }

      p[0][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[1]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[1]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[1]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }

      p[0][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[2]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[2]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[2]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      printf("ERROR: type:%d of face:%d not known. Interrupt\n",face[nr].type,nr); 
      return(-1.);
    }

    /* determine the average node-value */
    if(anz->l)
    {
      if (face[nr].type == 7) n = 3;  /* TRI3  */
      else if (face[nr].type == 8) n = 6;  /* TRI6  */
      else if (face[nr].type == 9) n = 4;  /* QUAD4 */
      else if (face[nr].type == 10) n = 8; /* QUAD8 */
      else if (face[nr].type == 11) n = 2; /* beam2 */
      else if (face[nr].type == 12) n = 3; /* beam3 */
      else n=0;
      value=0.;
      for (j=0; j<n; j++)
      {
        if(sequenceFlag) value+=lcase[lcase_animList].dat[animList][face[nr].nod[j]];
        else value+=lcase[cur_lc].dat[cur_entity][face[nr].nod[j]];
      }
      sum_valAe+=value/n*(A-Abuf);
    }
  }
  if(anz->l) printf("AREA:%e  CENTER OF GRAVITY: %e %e %e AVERAGE-VALUE:%e\n", A,xcg/A,ycg/A,zcg/A, sum_valAe/A);
  else   printf("AREA:%e  CENTER OF GRAVITY: %e %e %e\n", A,xcg/A,ycg/A,zcg/A);

  if(valuestackFlag)
  {
    if(anz->l) n=5; else n=4;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    if(anz->l) sprintf(valuestack[valuestack_ptr++],"%e", sum_valAe/A );
    sprintf(valuestack[valuestack_ptr++],"%e", zcg/A );
    sprintf(valuestack[valuestack_ptr++],"%e", ycg/A );
    sprintf(valuestack[valuestack_ptr++],"%e", xcg/A );
    sprintf(valuestack[valuestack_ptr++],"%e", A );
    printf(" in inverse order written to stack\n");
  }

  return(A);
}



double pre_volu(char *setname)
{
  int   i,j,n;
  int   nr, setNr, massFlag=1, elFlag=0;
  int   istat[3]={0,0,0};
  double vol=0., mass=0., vole, masse, x[20],y[20],z[20];
  double xcg=0., ycg=0., zcg=0., xcge, ycge, zcge;
  double sum_valVe=0., value=0.;
  double xl[20][3], cg[3];
  char elty[MAX_LINE_LENGTH];

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return(-1);
  }

  /* code also used in pre_eprop() */
  for(i=0; i<set[setNr].anz_e; i++)
  {
    nr=set[setNr].elem[i];
    if(e_enqire[nr].type==1)
    {
      for(j=0; j<8; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D8");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //printf("vol:%e cg: %f %f %f\n", vole,xcge,ycge,zcge);
      //return(0);
    }
    else if(e_enqire[nr].type==4)
    {
      for(j=0; j<12; j++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[j]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[j]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[j]].nz* scale->w+scale->z;
      }
      for(n=16; n<20; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      for(n=12; n<16; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      strcpy(elty,"C3D20");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //return(0);
    }
    else if(e_enqire[nr].type==2)
    {
      for(j=0; j<6; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D6");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
    }
    else if(e_enqire[nr].type==5)
    {
      for(j=0; j<9; j++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[j]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[j]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[j]].nz* scale->w+scale->z;
      }
      for(n=12; n<15; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      for(n=9; n<12; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      strcpy(elty,"C3D15");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
    }
    else if(e_enqire[nr].type==3)
    {
      for(j=0; j<4; j++)
      {
        x[j]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      tetraeder_(&nr, istat, x,y,z, &vole, &xcge, &ycge, &zcge );
    }
    else if(e_enqire[nr].type==6)
    {
      for(j=0; j<10; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D10");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //return(0);
    }
    else
    {
      printf("ERROR: type:%d of elem:%d not known. Interrupt\n",e_enqire[nr].type,nr); 
      return(-1.);
    }

    /* determine the average node-value */
    if(anz->l)
    {
      /* check if the data of the specified lcase (Dataset) are already available */
      if(sequenceFlag)
      {
        if (!lcase[lcase_animList].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lcase_animList, anz, node, lcase )==-1) 
          {
            printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", lcase_animList+1); 
            return(0);
          }
          calcDatasets( lcase_animList, anz, node, lcase );
          recompileEntitiesInMenu(lcase_animList);
        }
      }
      else
      {
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

      if (e_enqire[nr].type == 1) n = 8;  /* HEXA8 */
      else if (e_enqire[nr].type == 2) n = 6;  /* PENTA6 */
      else if (e_enqire[nr].type == 3) n = 4;  /* TET4 */
      else if (e_enqire[nr].type == 4) n = 20; /* HEX20 */
      else if (e_enqire[nr].type == 5) n = 15; /* PENTA15 */
      else if (e_enqire[nr].type == 6) n = 10; /* TET10 */
      else if (e_enqire[nr].type == 7) n = 3;  /* TRI3  */
      else if (e_enqire[nr].type == 8) n = 6;  /* TRI6  */
      else if (e_enqire[nr].type == 9) n = 4;  /* QUAD4 */
      else if (e_enqire[nr].type == 10) n = 8; /* QUAD8 */
      else if (e_enqire[nr].type == 11) n = 2; /* BEAM */
      else if (e_enqire[nr].type == 12) n = 3; /* BEAM3 */
      else n=0;
      value=0.;
      for (j=0; j<n; j++)
      {
        if(sequenceFlag) value+=lcase[lcase_animList].dat[animList][e_enqire[nr].nod[j]];
        else value+=lcase[cur_lc].dat[cur_entity][e_enqire[nr].nod[j]];
      }
      sum_valVe+=value/n*(vole);
    }

    /*
       Berechnung globaler Größen:
    */

    /* get material properties */
    masse=-MAX_FLOAT;
    elFlag=-1;
    for(j=0; j<anz->sets; j++)
    {
      if((set[j].name!=(char *)NULL) && (!set[j].type) && (set[j].name[0]!='-') && (set[j].material>-1)) 
      {
        if(( getIndex(&set[j].elem,set[j].anz_e,nr) >-1) && (material[set[j].material].rho>-1.))
        {
          if(elFlag!=-1)
          {
            printf("ERROR: found material-definition for elem:%d in set:%s and %s\n", nr, set[j].name,set[elFlag].name );  
            break;
          }
          elFlag=j;
          masse= vole * material[set[j].material].rho;
          mass+= masse;
        }
      }
    }
    if (masse==-MAX_FLOAT)
    {
      massFlag=0;
      masse=vole;
    }
              
    vol+=vole;
    xcg+=xcge*masse;
    ycg+=ycge*masse;
    zcg+=zcge*masse;
  }
  if((mass!=0.)&&(!massFlag))
  {
    printf("ERROR: some elements have no material assigned, either none or all need a material reference\n");
  }
  else
  {
    if (massFlag)     printf("VOLUME:%e  MASS:%e CENTER OF GRAVITY: %e %e %e", vol,mass,xcg/mass,ycg/mass,zcg/mass);
    else printf("VOLUME:%e CENTER OF GRAVITY: %e %e %e", vol,xcg/vol,ycg/vol,zcg/vol);
    if(anz->l) printf(" AVERAGE-VALUE:%e\n", sum_valVe/vol); else printf("\n");
  }

  if(valuestackFlag)
  {
    if(anz->l) n=5; else n=4;
    if (massFlag) n++;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    if(anz->l) sprintf(valuestack[valuestack_ptr++],"%e", sum_valVe/vol );
    if (massFlag)
    {
      sprintf(valuestack[valuestack_ptr++],"%e", zcg/mass );
      sprintf(valuestack[valuestack_ptr++],"%e", ycg/mass );
      sprintf(valuestack[valuestack_ptr++],"%e", xcg/mass );
      sprintf(valuestack[valuestack_ptr++],"%e", mass );
    }
    else
    {
      sprintf(valuestack[valuestack_ptr++],"%e", zcg/vol );
      sprintf(valuestack[valuestack_ptr++],"%e", ycg/vol );
      sprintf(valuestack[valuestack_ptr++],"%e", xcg/vol );
    }
    sprintf(valuestack[valuestack_ptr++],"%e", vol );
    printf(" in inverse order written to stack\n");
  }
  return(vol);
}


void completeFacesByTolerance(int set_highl, int setNr, double qaddTol)
{
  register int i,j,jj,jjj,jjjj;
  int f,n,n2,k,l, equ;

  int sumMaster=0;

  int *facebuf;  // stores the identified faces which fulfill the criterion
  int *fchecked; // 1: if a certain face was already checked
  double **fnorm;  // average normal on the face
  double criterion; 
  double v1[3], v2[3], sprod;
  Nodes *norm;
  int   *sum_n, nmax;

  typedef struct {
    int sum;
    int *face;   // face which uses this node (lowest node-nr in face
  }Fnode;
  Fnode *fnode;  // field indexed by the lowest node-nr of a face


  criterion=qaddTol*PI/180.;

  /*
  Go over all master-faces and check the neighbours against the criterion of qaddTol.
  If a face passes it will also be a master face.
  The ammount of masters will therefore increase up to a certain level.
  The comparison stops if no master is left.
  */

  if ( ( fnode= (Fnode *)malloc((anz->nmax+1) * sizeof(Fnode))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  for(i=0; i<=anz->nmax; i++) { fnode[i].face=NULL; fnode[i].sum=0; }
  if ( ( facebuf= (int *)malloc((set[setNr].anz_f+1) * sizeof(int))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  if ( ( fchecked= (int *)malloc((anz->f+1) * sizeof(int))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  if ( ( fnorm= (double **)malloc((anz->f+1) * sizeof(double *))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  for(i=0; i<anz->f; i++)
    if ( ( fnorm[i]= (double *)calloc((3), sizeof(double))) == NULL )
      printf("\n\n ERROR: realloc failed\n");
  for(i=0; i<anz->f; i++)
  {
    fchecked[i]=0;
  }
  for(i=0; i<set[setNr].anz_f; i++)
  {
    facebuf[i]=set[setNr].face[i];
    fchecked[set[setNr].face[i]]=1;
  }
  sumMaster=set[setNr].anz_f;

  /* calc the average normal on all faces */

  /* calculate the average normal on every node */
  getNodeNormalen(&sum_n, &norm, setall, anz, face);
  for (f=0; f<anz->f; f++)
  {
    if (face[f].type == 7) n = 3;  /* TRI3  */
    else if (face[f].type == 8) n = 3;  /* TRI6  */
    else if (face[f].type == 9) n = 4;  /* QUAD4 */
    else if (face[f].type == 10) n = 4; /* QUAD8 */
    else if (face[f].type == 11) n = 2; /* BEAM2 */
    else if (face[f].type == 12) n = 2; /* BEAM3 */
    else n=0;
    if(n==2)
    {
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[1]].nx, fnorm[f]);
    }
    if(n==3)
    {
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[1]].nx, v1);
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[2]].nx, v2);
      v_prod( v1, v2, fnorm[f] );
    }
    if(n==4)
    {
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[2]].nx, v1);
      v_result( &node[face[f].nod[1]].nx, &node[face[f].nod[3]].nx, v2);
      v_prod( v1, v2, fnorm[f] );
    }
    v_norm(fnorm[f],fnorm[f]);

    /* get the faces at each node */
    for(j=0;j<n; j++)
    {
      nmax=face[f].nod[j];
      if ( ( fnode[nmax].face= (int *)realloc((int *)fnode[nmax].face, (fnode[nmax].sum+1)*sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed\n");
      fnode[nmax].face[fnode[nmax].sum]=f;
      fnode[nmax].sum++;
    }
  }

  /* go over all master-faces and search and check the slaves */
  for(i=0; i<sumMaster; i++)
  {
    f=facebuf[i];

    /* nodes of the master */
    if (face[f].type == 7) n = 3;  /* TRI3  */
    else if (face[f].type == 8) n = 3;  /* TRI6  */
    else if (face[f].type == 9) n = 4;  /* QUAD4 */
    else if (face[f].type == 10) n = 4; /* QUAD8 */
    else if (face[f].type == 11) n = 2; /* BEAM2 */
    else if (face[f].type == 12) n = 2; /* BEAM3 */
    else n=0;

    for(jjj=0;jjj<n; jjj++)
    {
     nmax=face[f].nod[jjj];
     /* identify the neighbour faces */
     for (jj=0; jj<fnode[nmax].sum; jj++)
     {
      j=fnode[nmax].face[jj];
      if (face[j].type == 7) n2 = 3;  /* TRI3  */
      else if (face[j].type == 8) n2 = 3;  /* TRI6  */
      else if (face[j].type == 9) n2 = 4;  /* QUAD4 */
      else if (face[j].type == 10) n2 = 4; /* QUAD8 */
      else if (face[j].type == 11) n2 = 2; /* BEAM2 */
      else if (face[j].type == 12) n2 = 2; /* BEAM3 */
      else n2=0;
      equ=0;
      for (k=0; k<n2; k++) { for (l=0; l<n; l++)  if(face[f].nod[l] == face[j].nod[k]) equ++; }
      //if(j==91472) printf("i:%d master:%d m:%d s:%d equ:%d check:%d\n", i, sumMaster, f, j, equ, fchecked[j]);
      if((equ==2) || ((n2==2)&&(equ==1)))
      {
        /* check j against the criterion */
        sprod=abs(v_sprod(fnorm[f], fnorm[j]));
        if(sprod>1.) sprod=1.;
	/*
	if(j==91472){
	  printf("i:%d sum:%d S:%f angle %f between %d %d, criterion:%f  check:%d\n", i, sumMaster, sprod, acos(sprod), f, j, criterion, fchecked[j]);
	printf("norm master:%f neighbor: %f \n", fnorm[f][0], fnorm[j][0]);
	printf("norm master:%f neighbor: %f \n", fnorm[f][1], fnorm[j][1]);
	printf("norm master:%f neighbor: %f \n", fnorm[f][2], fnorm[j][2]);
	}
	*/
        if((!fchecked[j])&&(acos(sprod) < criterion))
	{
          /* add j as additional masterface */
          if ( ( facebuf= (int *)realloc((int *)facebuf, (sumMaster+1) * sizeof(int))) == NULL )
            printf("\n\n ERROR: realloc failed\n");
          facebuf[sumMaster]=j;
          sumMaster++; 
          fchecked[j]=1;
    printf ("%d e:%d s:%d n= ", j, face[j].elem_nr, face[j].nr+1 );
      if(face[j].type==7) k=3;
      else if(face[j].type==8) k=6;
      else if(face[j].type==9) k=4;
      else if(face[j].type==10) k=8;
      else k=0;
      for (jjjj=0; jjjj<k; jjjj++) printf("%d ",face[j].nod[jjjj]);
      printf("\n"); 
          seta( setNr, "f", j );
          seta(set_highl, "f", j );
	}
      }
     }
    }
  }
  free(facebuf);
  free(fchecked);
  free(norm);
  free(sum_n);
  for(i=0; i<anz->nmax; i++) free(fnode[i].face);
  free(fnode);
  for(i=0; i<anz->f; i++) free(fnorm[i]);
  free(fnorm);
}

/* return sum of generated sets */
int separateMeshes( char *setName, char *grpName)
{
  int i,e,k=2;
  int setNr, setNrbuf, setTmp, sum_c=0, c1, sum1,sum2;
  int *elUsed;
  char setname[MAX_LINE_LENGTH];
  char buffer[MAX_LINE_LENGTH];

  printf (" please wait for 'ready'\n");

  setNr=getSetNr(setName);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setName);
    return(0);
  }

  if (set[setNr].anz_e<2)
  {
    //printf("ERROR: set:%s contains less than 2 elements!\n", set[setNr].name);
    return(0);
  }
  /*----- complete set with all elements and nodes ---*/

  /* mark all elements as unused */
  if((elUsed = (int *)malloc( (int)(anz->emax+1)*sizeof(int)))==NULL)
  { errMsg("\n\n ERROR: malloc failed for elUsed\n" ); return(0); }
  for(i=0; i<=anz->emax; i++) elUsed[i]=1;
  for(i=0; i<set[setNr].anz_e; i++) elUsed[set[setNr].elem[i]]=0;

  /* use "comp set up/do" until no more entities are added */
  e=set[setNr].elem[0];
  do
  {
    sprintf( setname, "%s%d", grpName,sum_c+1);
    delSet(setname);
    c1=pre_seta(setname,"i", 0);
    seta(c1,"e", e);                                            // 1st elem in c1
    elUsed[e]=1; 

    delSet("+buf");
    sprintf(buffer,"%d", e);
    setNrbuf=pre_seta("+buf","e", buffer);
    completeSet_Mesh(  setNrbuf, 0, elUsed, 1);                 // down, nodes from new elems in +buf
    for(i=0; i<set[setNrbuf].anz_n; i++)
    {
      seta(c1,"n", set[setNrbuf].node[i]);                      // nodes from new elem in c1
    }

    do
    {
      sum1=set[c1].anz_n+set[c1].anz_e;

      delSet("+buf2");
      setTmp=pre_seta("+buf2", "i", 0 );
      completeSet_Mesh( setNrbuf, setTmp, elUsed, 0);                // up, only new elems from +buf (setNrbuf) in +buf2 (setTmp)
      completeSet_Mesh(  setTmp, 0, elUsed, 1);                      // down, nodes from new elems in +buf2 (setTmp)
      for(i=0; i<set[setNrbuf].anz_n; i++)
        setr(setTmp,"n", set[setNrbuf].node[i]);              // remove used nodes in +buf from +buf2
      for(i=0; i<set[setTmp].anz_e; i++)
      {
        seta(c1,"e", set[setTmp].elem[i]);                           // new elems in c1
        elUsed[set[setTmp].elem[i]]=1; 
      }

      delSet("+buf");
      setNrbuf=pre_seta("+buf","i", 0);
      for(i=0; i<set[setTmp].anz_n; i++)
      {
        seta(c1,"n", set[setTmp].node[i]);
        seta(setNrbuf,"n", set[setTmp].node[i]);
      }
      sum2=set[c1].anz_n+set[c1].anz_e;
    }while(sum1<sum2);

    //completeSet( setname, "f");                  // faces (draw-nodes are missed)
    completeSet( setname, "do");                  // faces, nodes (to include also the additional midface-nodes for drawing)

    if(k>=entitycols) k=BAS_COLS-1;
    printf("set%d: %s col:%s\n",sum_c+1, setname, entitycol[k].name);
    sprintf(buffer,"f %s %s", setname, entitycol[k++].name);
    if(!sum_c) plot(buffer);
    else plus(buffer);

    sum_c++;
    e=0;
    for(i=0; i<set[setNr].anz_e; i++)
    {
      if(elUsed[set[setNr].elem[i]]==0)
      {
        e=set[setNr].elem[i];
        break;
      }
    }
  }while(e);

  free(elUsed);
  delSet("+buf");
  delSet("+buf2");

  printf (" ready\n");
  return(sum_c);
}




void separateLines( char *setName, char *grpName)
{
  int i,e,k=2;
  int setNr, setNrbuf, setTmp, sum_c=0, c1, sum1,sum2;
  int *elUsed;
  char setname[MAX_LINE_LENGTH];
  char buffer[MAX_LINE_LENGTH];

  printf (" please wait for 'ready'\n");

  setNr=getSetNr(setName);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setName);
    return;
  }

  if (set[setNr].anz_l<2)
  {
    //printf("ERROR: set:%s contains less than 2 lines!\n", set[setNr].name);
    return;
  }
  /*----- complete set with all lines and points ---*/

  /* mark all lines as unused */
  if((elUsed = (int *)malloc( (int)(anzGeo->l+1)*sizeof(int)))==NULL)
  { errMsg("\n\n ERROR: malloc failed for elUsed\n" ); return; }
  for(i=0; i<=anzGeo->l; i++) elUsed[i]=1;
  for(i=0; i<set[setNr].anz_l; i++) elUsed[set[setNr].line[i]]=0;

  /* use "comp set up/do" until no more entities are added */
  e=set[setNr].line[0];
  do
  {
    sprintf( setname, "%s%d", grpName,sum_c+1);
    delSet(setname);
    c1=pre_seta(setname,"i", 0);

    delSet("+buf");
    setNrbuf=pre_seta("+buf","l", line[e].name);
    completeSet_Lines(  setNrbuf, 0, elUsed, 1);                      // down, nodes from new elems in +buf
    for(i=0; i<set[setNrbuf].anz_p; i++)
    {
      seta(c1,"p", set[setNrbuf].pnt[i]);
    }

    seta(c1,"l", e);
    elUsed[e]=1; 
    do
    {
      sum1=set[c1].anz_p+set[c1].anz_l;

      delSet("+buf2");
      setTmp=pre_seta("+buf2", "i", 0 );
      completeSet_Lines( setNrbuf, setTmp, elUsed, 0);                  // up, only new elems in +buf2 (setTmp) based on nodes in +buf (setNrbuf)

      completeSet_Lines(  setTmp, 0, elUsed, 1);                      // down, nodes from new elems in +buf2 (setTmp)

      for(i=0; i<set[setTmp].anz_l; i++)
      {
        seta(c1,"l", set[setTmp].line[i]);
        elUsed[set[setTmp].line[i]]=1; 
      }

      delSet("+buf");
      setNrbuf=pre_seta("+buf","i", 0);
      for(i=0; i<set[setTmp].anz_p; i++)
      {
        seta(c1,"p", set[setTmp].pnt[i]);
        seta(setNrbuf,"p", set[setTmp].pnt[i]);
      }
      sum2=set[c1].anz_p+set[c1].anz_l;
    }while(sum1<sum2);

    //completeSet( setname, "do");                  // faces, nodes (to include also the additional midface-nodes for drawing)

    if(k>=SET_COLS) k=BAS_COLS-1;
    printf("set%d: %s col:%s\n",sum_c+1, setname, entitycol[k].name);
    sprintf(buffer,"l %s %s", setname, entitycol[k++].name);
    if(!sum_c) plot(buffer);
    else plus(buffer);

    sum_c++;
    e=0;
    for(i=0; i<set[setNr].anz_l; i++)
    {
      if(elUsed[set[setNr].line[i]]==0)
      {
        e=set[setNr].line[i];
        break;
      }
    }
  }while(e);

  free(elUsed);
  delSet("+buf");
  delSet("+buf2");

  printf (" ready\n");
}


void pre_contact( char *record)
{
  int i,j;
  int length, setNr, sum_c, c1, c2, mpcset[2], flag;
  char setname[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH], par1[MAX_LINE_LENGTH], par2[MAX_LINE_LENGTH];
  double tol=0.,gtolbuf;
  double  stiffness=-1;
  double  mue=0.;
  FILE *handle=NULL;

  par1[0]=0;
  action[0]=0;
  length=sscanf(record, "%s %lf %s %s %s %s", setname, &tol, format, action, par1, par2);
  if(action[0]=='e')
  {
    if(length==5)
    {
      /* equations */
      if((par1[0]=='u')||(par1[0]=='c')) strcpy(par2,par1);
      //if(par1[0]!='t') strcpy(par1,"123");
    }
    else if(length>5)
    {
      /* equations */
      //if(par1[0]!='t') strcpy(par1,"123");
      if((par1[0]=='u')||(par1[0]=='c')) strcpy(par2,par1);
    }
    else
    {
      strcpy(par1,"123");
      strcpy(par2,"c");
    }
  }
  if(action[0]=='t')
  {
    if(length>=5)
    {
      if((par1[0]=='y')||(par1[0]=='Y')) strcpy(par1,"YES");
    }
    else
    {
      strcpy(par1,"NO");
    }
  }
  if((action[0]=='c')||(action[0]=='n'))
  {
    if(length>=5)
    {
      stiffness=atof(par1);
    }
    if(length>=6)
    {
      mue=atof(par2);
    }
  }
    
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return;
  }

  if (set[setNr].anz_f<2)
  {
    printf("ERROR: set:%s contains less than 2 faces. Contact between two faces not possible. Consider a <comp %s do> command.\n",set[setNr].name, set[setNr].name);
    return;
  }
  printf("Contact between faces of set:%s with tol:%lf\nwait for <ready>\n", set[setNr].name, tol);


  /* subdivide the faces into single unconnected sets of faces */
  /* return if not at least 2 indep meshes exist */
  sum_c=getMeshSections(setNr, anz, face, node);
  if(sum_c < 2) 
  {
    printf("ERROR: Found only %d separate meshes. Contact not possible\n", sum_c);
    return;
  }
  printf(" found %d separate meshes\n", sum_c);


  /* search face-pairs of unconnected sets of faces */
  /* deside which side is the dep side */
  /* remove all dep-nodes which are not inside the ind-faces */

  if((action[0]=='c')||(action[0]=='n')||(action[0]=='t'))
  {
    if(compareStrings(format, "abq") !=3)
    {
      printf("ERROR: Format %s not supported so far for contatct based connections\n", format);
      return;
    }
    handle = fopen ("neigh.con", "w");
    if ( handle== NULL )
    {
      printf ("\nThe file 'neigh.con' could not be opened.\n\n"); 
      return;
    }
    else printf ("\n write file: neigh.con\n\n");

    if(action[0]=='c')
    {
      if(par1[0]=='t')
      {
        //fprintf(handle,"*SURFACE INTERACTION,NAME=SI%d%d\n",i,j+1);
        fprintf(handle,"*SURFACE INTERACTION,NAME=DEFAULT\n");
        fprintf(handle,"*SURFACE BEHAVIOR,PRESSURE-OVERCLOSURE=TIED\n");
        fprintf(handle,"1e7\n");
        fprintf(handle,"*FRICTION\n");
        fprintf(handle,"1.e30,1e7\n");
      }
      else if(stiffness!=-1)
      {
        //fprintf(handle,"*SURFACE INTERACTION,NAME=SI%d%d\n",i,j+1);
        fprintf(handle,"*SURFACE INTERACTION,NAME=DEFAULT\n");
        fprintf(handle,"*SURFACE BEHAVIOR,PRESSURE-OVERCLOSURE=LINEAR\n");
        fprintf(handle,"%e\n", stiffness);
        if(mue!=0.)
        {
          fprintf(handle,"*FRICTION\n");
          fprintf(handle,"%f,%e\n",mue,stiffness/100.);
        }
      }
    }
    if(action[0]=='n')
    {
      if(par1[0]=='t')
      {
        //fprintf(handle,"*SURFACE INTERACTION,NAME=SI%d%d\n",i,j+1);
        fprintf(handle,"*SURFACE INTERACTION,NAME=DEFAULT\n");
        //fprintf(handle,"*SURFACE BEHAVIOR,PRESSURE-OVERCLOSURE=TIED\n");
        fprintf(handle,"****************************************************\n");
        fprintf(handle,"** TIED not implemented for n2s in ccx:           **\n");
        fprintf(handle,"**   *SURFACE BEHAVIOR,PRESSURE-OVERCLOSURE=TIED  **\n");
        fprintf(handle,"**                                                **\n");
        fprintf(handle,"** The following lines are used to simulate TIED  **\n");
        fprintf(handle,"**   Run your calculation this way:               **\n");
        fprintf(handle,"**   ** generate springs                          **\n");
        fprintf(handle,"**   *STEP                                        **\n");
        fprintf(handle,"**   *STATIC                                      **\n");
        fprintf(handle,"**   *BOUNDARY                                    **\n");
        fprintf(handle,"**   Nall,1,3                                     **\n");
        fprintf(handle,"**   *END STEP                                    **\n");
        fprintf(handle,"**   ** keep the springs                          **\n");
        fprintf(handle,"**   *STEP,NLGEOM,perturbation                    **\n");
        fprintf(handle,"**   ** or:                                       **\n");
        fprintf(handle,"**   *STEP,perturbation                           **\n");
        fprintf(handle,"**   *STATIC                                      **\n");
        fprintf(handle,"**   *BOUNDARY,OP=NEW                             **\n");
        fprintf(handle,"**   ** or:                                       **\n");
        fprintf(handle,"**   *FREQUENCY                                   **\n");
        fprintf(handle,"**   *BOUNDARY,OP=NEW                             **\n");
        fprintf(handle,"**   ..                                           **\n");
        fprintf(handle,"****************************************************\n");
        fprintf(handle,"*SURFACE BEHAVIOR,PRESSURE-OVERCLOSURE=LINEAR\n");
        fprintf(handle,"1e7\n");
        fprintf(handle,"*FRICTION\n");
        fprintf(handle,"1.e30,1e7\n");
      }
      else if(stiffness!=-1)
      {
        //fprintf(handle,"*SURFACE INTERACTION,NAME=SI%d%d\n",i,j+1);
        fprintf(handle,"*SURFACE INTERACTION,NAME=DEFAULT\n");
        fprintf(handle,"*SURFACE BEHAVIOR,PRESSURE-OVERCLOSURE=LINEAR\n");
        fprintf(handle,"%e\n", stiffness);
        if(mue!=0.)
        {
          fprintf(handle,"*FRICTION\n");
          fprintf(handle,"%f,%e\n",mue,stiffness/100.);
        }
      }
    }
  }

  /* ------- node-koordinaten berechnen und am ende wieder scalieren ------ */
  descalNodes ( anz->n, node, scale );

  delSet("+UNCON");
  for(i=1; i<sum_c; i++)
  {
    sprintf( setname, "%s%d", specialset->cf,i); 
    c1=getSetNr(setname);
    for(j=i; j<sum_c; j++)
    {
      sprintf( setname, "%s%d", specialset->cf,j+1); 
      c2=getSetNr(setname);

      flag=getFacePair( anz, c1, c2, e_enqire, face, node, tol, mpcset, par1);
      if(flag)
      {
        set[c1].eattr=1;
        set[c2].eattr=1;
        if(action[0]=='e')
	{
          /* write equations */
          /* write only dep-nodes which are not already used (1) */
          completeSet(set[mpcset[1]].name, "do") ;
	  gtolbuf=gtol;
          if(gtol<tol) gtol=tol;
	  //printf(" areampc %s tol:%f\n",format, gtol );
          areampc(mpcset[0], mpcset[1], format, "areampc", par1, par2, 1, node, 1, 0);
          gtol=gtolbuf;
	}
        if((action[0]=='c')&&(flag==2))
	{
          sendSurfaces( set[mpcset[0]].name, format, anz, node, e_enqire, NULL );
          sendSurfaces( set[mpcset[1]].name, format, anz, node, e_enqire, NULL );
          /* write contact definitions */
          //fprintf(handle,"*CONTACT PAIR,INTERACTION=SI%d%d,TYPE=SURFACE TO SURFACE\n",i,j+1);
          fprintf(handle,"*CONTACT PAIR,INTERACTION=DEFAULT,TYPE=SURFACE TO SURFACE\n");
          fprintf(handle,"S%s,S%s\n",set[mpcset[0]].name,set[mpcset[1]].name);
	}
        if((action[0]=='t')&&(flag==2))
	{
          sendSurfaces( set[mpcset[0]].name, format, anz, node, e_enqire, NULL );
          sendSurfaces( set[mpcset[1]].name, format, anz, node, e_enqire, NULL );
          /* write tie definitions */
          fprintf(handle,"*TIE,NAME=SI%d%d,ADJUST=%s\n",i,j+1,par1);
          fprintf(handle,"S%s,S%s\n",set[mpcset[0]].name,set[mpcset[1]].name);
	}
        if(action[0]=='n')
        {
          /* write node sets */
          sendNames( set[mpcset[0]].name, format, anz, node, e_enqire );
          sendSurfaces( set[mpcset[1]].name, format, anz, node, e_enqire, NULL );
          /* write contact definitions */
          fprintf(handle,"*SURFACE,NAME=S%s,TYPE=NODE\n",set[mpcset[0]].name);
          fprintf(handle,"N%s\n",set[mpcset[0]].name);
          fprintf(handle,"*CONTACT PAIR,INTERACTION=DEFAULT,TYPE=NODE TO SURFACE\n");
          fprintf(handle,"S%s,S%s\n",set[mpcset[0]].name,set[mpcset[1]].name);
	}
        strcpy(parameter[0],set[mpcset[0]].name);
        strcpy(parameter[1],set[mpcset[1]].name);
        write2stack(2, parameter);
      }
    }
  }
  if(handle!=NULL) fclose(handle);

  /* are there unconnected sets? */
  for(i=1; i<=sum_c; i++)
  {
    sprintf( setname, "%s%d", specialset->cf,i); 
    c1=getSetNr(setname);
    if(set[c1].eattr==0) pre_seta("+UNCON","se",set[c1].name);
    else set[c1].eattr=0;
  }
  scalNodes ( anz->n, node, scale );
  printf("ready\n");
}



void pre_del( char *record)
{
  int  b,bb, i, j,jj, k, l,ll, n,nn, m, s,ss, z;
  int length, setNr, nf=0, foundSubString, *lines;
  char type[MAX_LINE_LENGTH], **datum=NULL, **dat=NULL, *ori;
  char buffer[MAX_LINE_LENGTH];
  double  v01[3], ds;

  length= sword2( record, type );
  for (i=0; i<MAX_PARAM_PER_RECORD; i++)
  {
    if((datum=(char **)realloc((char **)datum, (i+1)*sizeof(char *)))==NULL)
      printf("\n\n ERROR: realloc failure\n\n" );
    if((datum[i]=(char *)malloc( (MAX_LINE_LENGTH)*sizeof(char)))==NULL)
      printf("\n\n ERROR: realloc failure\n\n" );
    n =  sword2( &record[length], datum[i] );
    if (n<1) break;
    length= length+n;
    operateAlias( datum[i], type );
  }

  delSet(specialset->zap);

  /* check if wildcards (*) were used */
  k=0;
  for (n=0; n<i; n++)
  {
    length= strsplt( datum[n], '*', &dat);
    if ((length>0)&&(strstr(datum[n], "*") !=NULL))
    {
     if (compare(type, "se", 2) == 2 )
     {
      for(setNr=0; setNr<anz->sets; setNr++)
      {
        if( set[setNr].name == (char *)NULL ) continue;

        foundSubString=0;
        for(l=0; l<length; l++)
        {
          if(strstr(set[setNr].name, dat[l]) !=NULL)
          {
            foundSubString++;
            /* check if the first or the last char is no '*' then the substring[] must be at start or end */
            if(l==0) { if(datum[n][0]!='*')  { if(datum[n][0]!=set[setNr].name[0])  foundSubString--; }  }
            if(l==length-1) { if(datum[n][strlen(datum[n])-1]!='*') { if(datum[n][strlen(datum[n])-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
          }
        }

        if(foundSubString==length)
        {
          printf(" set:%s matches %s\n", set[setNr].name, datum[n]);
          if((datum=(char **)realloc((char **)datum, (i+k+1)*sizeof(char *)))==NULL)
            printf("\n\n ERROR: realloc failure\n\n" );
          if((datum[i+k]=(char *)malloc( (MAX_LINE_LENGTH)*sizeof(char)))==NULL)
            printf("\n\n ERROR: realloc failure\n\n" );
          strcpy(datum[i+k],set[setNr].name);
          k++;
        }
      }
     }
     else printf(" ERROR in %s, wildcards '*' are not supported for type:%s\n", datum[n], type);
    }
  }
  i+=k;
  if(printFlag) for (n=0; n<i; n++) printf("datum:'%s' \n", datum[n]);


  if (compare(type, "se0", 3) == 3 ) /* delete empty sets */
  {
    for (n=0; n<anz->sets; n++)
    {
      if((set[n].name != (char *)NULL )&&
         (!set[n].anz_v)&&
         (!set[n].anz_n)&&
         (!set[n].anz_e)&&
         (!set[n].anz_f)&&
         (!set[n].anz_p)&&
         (!set[n].anz_l)&&
         (!set[n].anz_c)&&
         (!set[n].anz_s)&&
         (!set[n].anz_b)&&
         (!set[n].anz_nurl)&&
         (!set[n].anz_nurs)&&
         (!set[n].anz_se)&&
	(!set[n].anz_sh)) { delSet(set[n].name); }
    }
    return;
  }
  else if ((compare(type, "se", 2) == 2 )||(compare(type, "sq", 2) == 2 ))
  {
    for (n=0; n<i; n++)
      delSet(datum[n]);
    return;
  }
  else if (compare(type, "sh", 2) == 2 )
  {
    for (n=0; n<i; n++)
    {
      j=getShapeNr(datum[n]);
      if(j>-1) delShape(1, &j);
      else printf(" WARNING: shape %s does not exist\n", datum[n]);
    }
    return;
  }
  else if (compare(type, "pic", 2) == 2 )
  {
    bgpicture=0;
    return;
  }
  else if(compare(type, "mesh", 2) == 2 )
  {
    /* delete the temporary sets which hold the dependent and independent nodes for incompatible elements */
    for(j=0; j<sum_equSets; j++) { delSet(set[depSet[j]].name); delSet(set[indSet[j]].name); }
    sum_equSets=0;

    /* delete the mesh */
    if(anz->n<1) return;
    if((i>0)&&(strlen(datum[i-1])>0)) 
    { 
      setNr=getSetNr(datum[i-1]);
      if(setNr<0)
      {
        printf(" ERROR: set |%s| does not exist\n",datum[i-1]);
        return;
      }
    }
    else setNr=0;
    if(printFlag) printf ("delete mesh defined in set:%s\n", set[setNr].name);
 
    if(setNr==getSetNr("all"))
    {
      for (j=0; j<anz->sets; j++ )
      {
        set[j].anz_n = 0;
        set[j].anz_e = 0;
        set[j].anz_f = 0;
      }
      for (j=0; j<anzGeo->psets; j++ )
      {
        set[pset[j].nr].anz_n=0;
        set[pset[j].nr].anz_e=0;
        set[pset[j].nr].anz_f=0;
      }

      for (j=0; j<anz->l; j++)
      {
        freeDataset(lcase, j);
      }
      free(lcase); lcase=NULL;

      for (j=0; j<anz->e; j++)
      {
        /* free space for the normal-vectors */
        if(e_enqire[e_enqire[j].nr].side!=NULL)
        {
          if (e_enqire[e_enqire[j].nr].type == 1)       nf=6;  /* HEXA8 */
          else if (e_enqire[e_enqire[j].nr].type == 2)  nf=6;  /* PENTA6 */
          else if (e_enqire[e_enqire[j].nr].type == 3)  nf=4;  /* TET4 */
          else if (e_enqire[e_enqire[j].nr].type == 4)  nf=48; /* HEXA20 */
          else if (e_enqire[e_enqire[j].nr].type == 5)  nf=48; /* PENTA15 */
          else if (e_enqire[e_enqire[j].nr].type == 6)  nf=16; /* TET10 */
          else if (e_enqire[e_enqire[j].nr].type == 7)  nf=1;  /* TRI3  */
          else if (e_enqire[e_enqire[j].nr].type == 8)  nf=4; /* TRI6  */
          else if (e_enqire[e_enqire[j].nr].type == 9)  nf=2; /* QUAD4 */
          else if (e_enqire[e_enqire[j].nr].type == 10) nf=8; /* QUAD8 */
          else if (e_enqire[e_enqire[j].nr].type == 11) nf=1; /* BEAM */
          else if (e_enqire[e_enqire[j].nr].type == 12) nf=1; /* BEAM3 */
          for(k=0; k<nf; k++) free(e_enqire[e_enqire[j].nr].side[k]);
          free(e_enqire[e_enqire[j].nr].side);
          e_enqire[e_enqire[j].nr].side=NULL;
        }
      }
      for (j=0; j<anz->f; j++)
      {
        if(face[j].side!=NULL)
        {
          if (face[j].type == 7)       nf=1;  /* TRI3  */
          else if (face[j].type == 8)  nf=4; /* TRI6  */
          else if (face[j].type == 9)  nf=2; /* QUAD4 */
          else if (face[j].type == 10) nf=8; /* QUAD8 */
          else if (face[j].type == 11) nf=1; /* BEAM */
          else if (face[j].type == 12) nf=1; /* BEAM3 */
          for(k=0; k<nf; k++) free(face[j].side[k]);
          free(face[j].side);
          face[j].side=NULL;
        }
      }
      if(anz->p)
      {
        for(i=0; i<anz->p; i++) free(anz->pheader[i]);
        free(anz->pheader);
        anz->p=0;
      }

      anz->n=0;
      anz->e=0;
      anz->g=0;
      anz->f=0;
      anz->l=0;
      anz->nmax=0;
      anz->nmin=MAX_INTEGER;
      anz->emax=0;
      anz->emin=MAX_INTEGER;
      anz->orignmax=anz->orign=anz->olc=0;
      anz->nnext=1;
      anz->enext=1;

      for (j=0; j<anzGeo->p; j++)
      { free(point[j].nod);
        point[j].nod=NULL; 
        point[j].nn=0; }
      for (j=0; j<anzGeo->l; j++)
      { free(line[j].nod); free(line[j].elem);
        line[j].nod=NULL; line[j].elem=NULL; 
        line[j].nn=0; line[j].ne=0; }
      for (j=0; j<anzGeo->s; j++)
      { surf[j].nn=0; free(surf[j].nod); surf[j].nod=NULL;
        surf[j].ne=0; free(surf[j].elem); surf[j].elem=NULL; }
      for (j=0; j<anzGeo->b; j++)
      { body[j].nn=0; free(body[j].nod); body[j].nod=NULL;
        body[j].ne=0; free(body[j].elem); body[j].elem=NULL; }
      if(drawMode!=4) plot("w all");
      updateDispLists();
      createNewMainMenu();
    }
    else
    {
      j=pre_seta( specialset->zap, "i", 0 );
      for (k=0; k<set[setNr].anz_n; k++) seta(j, "n",set[setNr].node[k]); 
      for (k=0; k<set[setNr].anz_e; k++) seta(j, "e",set[setNr].elem[k]); 
      if( zap(specialset->zap)==-1)
         printf (" ERROR in ZAP: set:%s does not exist\n",specialset->zap );
    }
    return;
  }
  else if ((compare(type, "l0", 2) == 2 )&&(strlen(type)==2))  /* delete lines of zero length */
  {
    setNr=getSetNr(datum[0]);
    if(setNr<0)
    {
      printf(" ERROR: set %s does not exist\n",datum[0]);
      return;
    }

  searchNewZeroLines:;
    delSet("-mrgp");
    delSet(specialset->tmp);
    m=pre_seta("-mrgp","i", 0);
    k=pre_seta(specialset->tmp,"i", 0);
    z=pre_seta(specialset->zap,"i", 0);

    for(i=0; i<set[setNr].anz_l; i++)
    {
      j=set[setNr].line[i];
      if( line[j].name == (char *)NULL ) continue;

      /* check the length of the line */
      v_result( &point[line[j].p1].px, &point[line[j].p2].px, v01 );
      ds=v_betrag( v01 );
      if(ds==0.)
      {
        seta(k, "l",j);
        seta(z, "l",j);
      }
    }

    if(set[z].anz_l)
    {
      completeSet(specialset->tmp,"do");
      sprintf(buffer,"p %s n",specialset->tmp);
      pre_merge(buffer);
  
      /* delete the lines */
      for(i=0; i<set[z].anz_l; i++)
      {
        j=set[z].line[i];
        /* suche abhaengige lcmbs */
        for (jj=0; jj<anzGeo->c; jj++)
        {
          if( lcmb[jj].name != (char *)NULL )
          for (n=0; n<lcmb[jj].nl; n++)
          {
            if( j == lcmb[jj].l[n] )
            {
              if ((lines = (int *)malloc( (lcmb[jj].nl)*sizeof(int)) ) == NULL )
              { printf("ERROR: malloc failure\n"); return; }
              if ((ori = (char *)malloc( (lcmb[jj].nl)*sizeof(char)) ) == NULL )
              { printf("ERROR: malloc failure\n"); return; }
              l=0;
              if(lcmb[jj].nl>1)
  	      {        
                for (ll=0; ll<lcmb[jj].nl; ll++)
                {
                  if(lcmb[jj].l[ll]!=j) { lines[l]= lcmb[jj].l[ll]; ori[l++]=lcmb[jj].o[ll]; }
                  else if(!ll) line[lcmb[jj].l[1]].div+=line[j].div;
                  else line[lcmb[jj].l[ll-1]].div+=line[j].div;
                }
                getNewName( buffer, "c" );
                l=lcmb_i( buffer, 0, lcmb[jj].nl-1, ori, lines );
                free(lines);
                free(ori);
  	      }
              else l=-1;
  
              if (l >=0 )
  	      {
                /* keep the original lcmb since the new lcmb was added to eventually open sets */
                free(lcmb[jj].l);
                free(lcmb[jj].o);
                lcmb[jj].l=lcmb[l].l;
                lcmb[jj].o=lcmb[l].o;
                lcmb[jj].nl=lcmb[l].nl;
                lcmb[jj].p1=lcmb[l].p1;
                lcmb[jj].p2=lcmb[l].p2;
                free(lcmb[l].name);
                lcmb[l].name = (char *)NULL;
  	      }
              else
              {
                printf(" ERROR in del: lcmb could not be redefined\n");
  
                /* suche abhaengige surfs */
                for (s=0; s<anzGeo->s; s++)
                {
                  if( surf[s].name != (char *)NULL )
                  for (nn=0; nn<surf[s].nl; nn++)
                  {
                    if(( jj == surf[s].l[nn] )&&( surf[s].typ[nn] == 'c' ))
                    {
                      l=0;              
                      for (ll=0; ll<surf[s].nl; ll++)
                      {
                        if(surf[s].l[ll]!=jj)
                        {
                          surf[s].l[l]=surf[s].l[ll];
                          surf[s].o[l]=surf[s].o[ll];
                          surf[s].typ[l]=surf[s].typ[ll];
                          l++;
                        }
  		        else
  		        {
                          if(surf[s].typ[ll]=='l') { seta(m,"p", line[surf[s].l[ll]].p1); seta(m,"p", line[surf[s].l[ll]].p2); }
                          if(surf[s].typ[ll]=='c') { seta(m,"p", lcmb[surf[s].l[ll]].p1); seta(m,"p", lcmb[surf[s].l[ll]].p2); }
  	                }
                      }
                      if (l <=0 )
                      {
                        printf(" WARNING: surf:%s is deleted\n", surf[s].name);

                        /* suche abhaengige bodies */
                        for (bb=0; bb<anzGeo->b; bb++)
                        {
                          if( body[bb].name != (char *)NULL )
                          for (b=0; b<body[bb].ns; b++)
                          {
                            if( jj == body[bb].s[b] )
                            {
                              ss=0;              
                              for (ll=0; ll<body[bb].ns; ll++)
                              {
                                if(body[bb].s[ll]!=jj)
                                {
                                  body[bb].s[ss]=body[bb].s[ll];
                                  body[bb].o[ss]=body[bb].o[ll];
                                  ss++;
                                }
			      }
                              body[bb].ns=ss;
                              break;
			    }
			  }
			}

                        sprintf(buffer,"s %s", surf[s].name);
                        pre_del(buffer);
  		      }
                      else surf[s].nl=l;
                      break;
                    }
                  }
                }
              }
              break;
            }
          }
        }
  
        /* suche abhaengige surfs */
        for (jj=0; jj<anzGeo->s; jj++)
        {
          if( surf[jj].name != (char *)NULL )
          for (n=0; n<surf[jj].nl; n++)
          {
            if(( j == surf[jj].l[n] )&&( surf[jj].typ[n] == 'l' ))
            {
              l=0;              
              for (ll=0; ll<surf[jj].nl; ll++)
              {
                if(surf[jj].l[ll]!=j)
                {
                  surf[jj].l[l]=surf[jj].l[ll];
                  surf[jj].o[l]=surf[jj].o[ll];
                  surf[jj].typ[l]=surf[jj].typ[ll];
                  l++;
                }
  	        else
  	        {
                  if(surf[jj].typ[ll]=='l') { seta(m,"p", line[surf[jj].l[ll]].p1); seta(m,"p", line[surf[jj].l[ll]].p2); }
                  if(surf[jj].typ[ll]=='c') { seta(m,"p", lcmb[surf[jj].l[ll]].p1); seta(m,"p", lcmb[surf[jj].l[ll]].p2); }
  		}
              }
              if (l <=0 )
              {
                printf(" WARNING: surf:%s is deleted\n", surf[jj].name);

                /* suche abhaengige bodies */
                for (bb=0; bb<anzGeo->b; bb++)
                {
                  if( body[bb].name != (char *)NULL )
                  for (b=0; b<body[bb].ns; b++)
                  {
                    if( jj == body[bb].s[b] )
                    {
                      ss=0;              
                      for (ll=0; ll<body[bb].ns; ll++)
                      {
                        if(body[bb].s[ll]!=jj)
                        {
                          body[bb].s[ss]=body[bb].s[ll];
                          body[bb].o[ss]=body[bb].o[ll];
                          ss++;
                        }
		      }
                      body[bb].ns=ss;
                      break;
		    }
		  }
		}

                sprintf(buffer,"s %s", surf[jj].name);
                pre_del(buffer);
  	      }
              else surf[jj].nl=l;
              break;
            }
          }
        }
      }
    }
    if(set[m].anz_p)
    {
      pre_merge("p -mrgp 0.");
      // new zero length lines could exist
      goto searchNewZeroLines;
    }
    zap(specialset->zap);
    delSet("-mrgp");
    return;
  }
  else if (compare(type, "t", 1) == 1 )
  {
    setNr=getSetNr(datum[0]);
    if(setNr<0)
    {
      printf(" ERROR: set |%s| does not exist\n",set[setNr].name);
      return;
    }
    for(i=0; i<set[setNr].anz_n; i++)
    {
      for(j=0; j<anz->t; j++)
        if(ntext[j].node_nr==set[setNr].node[i]) ntext[j].node_nr=0;
    }
    return;
  }
  else if (((compare(type, "n", 1) == 1 )||
      (compare(type, "e", 1) == 1 )||
      (compare(type, "p", 1) == 1 )||
      (compare(type, "l", 1) == 1 )||
      (compare(type, "c", 1) == 1 )||
      (compare(type, "s", 1) == 1 )||
      (compare(type, "v", 1) == 1 )||
      (compare(type, "b", 1) == 1 )||
      (compare(type, "S", 1) == 1 ))&&(strlen(type)==1)) 
  {
    for (n=0; n<i; n++) pre_seta( specialset->zap, type, datum[n] );
    if( zap(specialset->zap)==-1)
          printf (" ERROR in ZAP: set:%s does not exist\n",specialset->zap );
    return;
  }
  else printf(" ERROR: type %s not known\n", type);
}


void pre_bia( char *record)
{
  int  j=0, nr, setNr;
  char setname[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH];
  double factor=1., value=1.;
  double bias,div;
  int bias_fbd;     /* bias * 10 to gain one decimal digit (bias 10 == 100) */

  sscanf( record, "%s%s%lf%lf", setname, type, &factor, &value );

  if ((type[0]=='M')||(type[0]=='m')||(type[0]=='D')||(type[0]=='d') )
  {
    if ((type[0]=='D')||(type[0]=='d') ) factor=1./factor;
    operateAlias( setname, "se" );
    nr=getSetNr( setname );
    if(nr<0)
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
    for (j=0; j<set[nr].anz_l; j++)
    {
      div=line[set[nr].line[j]].div;
      bias=line[set[nr].line[j]].bias;
      if(bias<1.)
      {
        bias_fbd= (pow(1./bias,(div-1.))*-10.)-.5;
        bias_fbd*=factor;
        if(bias_fbd>-10) line[set[nr].line[j]].bias=1.;
        else line[set[nr].line[j]].bias= 1./pow(((double)bias_fbd*-0.1), (1./((double)div-1.)));
      }
      else if(bias>1.)
      {
        bias_fbd= (pow(bias,(div-1.))*10)+.5;
        bias_fbd*=factor;
        if(bias_fbd<10) line[set[nr].line[j]].bias=1.;
        else line[set[nr].line[j]].bias=pow(((double)bias_fbd*0.1), (1./((double)div-1.)));
      }
    }
  }
  else if((type[0]>='1')&&(type[0]<='9'))
  {
    operateAlias( setname, "l" );
    nr=getLineNr( setname );

    operateAlias( setname, "se" );
    setNr=getSetNr( setname );

    /* its a line */
    if(nr>-1)
    {
      div=line[nr].div;
      bias=atof( &type[0])*factor;
      if(div>1)
      {
        if(bias<0) bias= 1./pow((bias*-1), (1./((double)div-1.)));
        else       bias=pow(bias, (1./((double)div-1.)));
      }
      else bias=1.;
      line[nr].bias=bias;
      repLine(nr);
    }
    /* its a set */
    else if(setNr>-1)
    {
      for (j=0; j<set[setNr].anz_l; j++)
      {
        div=line[set[setNr].line[j]].div;
        if(div>1)
        {
          bias=atof(type)*factor;
          if(bias<0) bias= 1./pow((bias*-1), (1./((double)div-1.)));
          else       bias=pow(bias, (1./((double)div-1.)));
        }
        else bias=1.;
	printf("bias:%f factor:%f\n", bias,factor);
        line[set[setNr].line[j]].bias=bias;
      }
      for (j=0; j<set[setNr].anz_l; j++) repLine(set[setNr].line[j]);
    }
    else
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
  }
  else
  {
    errMsg(" ERROR: command:|%s| not recognized\n", record );
  }
  return;
}


void pre_div( char *record)
{
  int  length,j=0, nr, setNr;
  char setname[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH];
  double factor=1., value=1., bias=1., length_ratio=ELEM_LENGTH_RATIO;
  int bias_fbd,div;

  autoDivFlag=0;
  length= sscanf( record, "%s%s%lf%lf%lf", setname, type, &factor, &value, &length_ratio);

  if(length<1)
  {
    printf(" default:%d\n auto mode: lmax:%e angle:%.1f ratio:%.3f\n",ddiv,gtol*GTOL_NODE_DIST,acos(GTOL_COS_A)*180./PI, length_ratio);
    return;
  }

  if((length==1)&&(atoi(setname)>0))
  {
    printf("DDIV %s\n",setname);
    sprintf(type,"DDIV %s",setname);
    pre_proc(type);
  }
  else if ((length>2)||(compare(type, "auto", 1) == 1 ))
  {
    /* its a set */
    operateAlias( setname, "se" );
    nr=getSetNr( setname );
    if(nr<0)
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
    if ((type[0]=='A')||(type[0]=='a') )
    {
      if(length_ratio>1.)
      {
        printf(" ERROR: element length ratio must be lower than 1 but is:%f\n", length_ratio  );
        return;
      }
      if(length==2) { value=GTOL_COS_A; factor=gtol*GTOL_NODE_DIST/scale->w; }
      else if(length==3) { value=GTOL_COS_A; factor/=scale->w; }
      else  { value=cos(value*PI/180.); factor/=scale->w; }
      for (j=0; j<set[nr].anz_l; j++)
      {
	if(line[set[nr].line[j]].name!=NULL)
          calcLineDiv(line, set[nr].line[j], value, factor, factor*length_ratio  ); 
      }
    }
    else if ((type[0]=='M')||(type[0]=='m') )
    {
      for (j=0; j<set[nr].anz_l; j++) if(line[set[nr].line[j]].name!=NULL)
      {
        if((line[set[nr].line[j]].bias!=1.)&&(line[set[nr].line[j]].div>1))
	{
          if(line[set[nr].line[j]].bias<1.)
	  {
            bias_fbd= (int)((pow(1./line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*-10.) -0.5) * value;
	  }
          else
	  {
            bias_fbd= (int)((pow(line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*10.) +0.5) * value;
	  }
          line[set[nr].line[j]].div*=factor;
          if(bias_fbd*bias_fbd<10.) line[set[nr].line[j]].bias=1.;
          else
          {
            if(bias_fbd<0) line[set[nr].line[j]].bias= 1./pow((bias_fbd*-.1), (1./((double)line[set[nr].line[j]].div-1.)));
            else       line[set[nr].line[j]].bias=pow((bias_fbd*.1), (1./((double)line[set[nr].line[j]].div-1.)));
          }
        }
        else line[set[nr].line[j]].div*=factor;
      }
    }
    else if ((type[0]=='D')||(type[0]=='d') )
    {
      for (j=0; j<set[nr].anz_l; j++) if(line[set[nr].line[j]].name!=NULL)
      {
        if((line[set[nr].line[j]].bias!=1.)&&(line[set[nr].line[j]].div>1))
	{
          if(line[set[nr].line[j]].bias<1.)
	  {
            bias_fbd= (int)((pow(1./line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*-10.) -0.5) / value;
	  }
          else
	  {
            bias_fbd= (int)((pow(line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*10.) +0.5) / value;
	  }
          line[set[nr].line[j]].div/=factor;
          if(bias_fbd*bias_fbd<10.) line[set[nr].line[j]].bias=1.;
          else
          {
            if(bias_fbd<0) line[set[nr].line[j]].bias= 1./pow((bias_fbd*-.1), (1./((double)line[set[nr].line[j]].div-1.)));
            else       line[set[nr].line[j]].bias=pow((bias_fbd*.1), (1./((double)line[set[nr].line[j]].div-1.)));
          }
        }
        else line[set[nr].line[j]].div/=factor;
        if(line[set[nr].line[j]].div<1) line[set[nr].line[j]].div=1;
        if(line[set[nr].line[j]].div==1) line[set[nr].line[j]].bias=1;
      }
    }
    else if (length==2)
    {
      div=atoi(type);
      splitBiasDiv(&div, &bias);
      line[set[nr].line[j]].div=div;
      line[set[nr].line[j]].bias=bias;
    }
    else printf(" ERROR: type %s not known\n", type);
  
    for (j=0; j<set[nr].anz_l; j++) repLine(set[nr].line[j]);
  }
  else if((length==2)&&(type[0]>='1')&&(type[0]<='9'))
  {
    operateAlias( setname, "l" );
    nr=getLineNr( setname );

    operateAlias( setname, "se" );
    setNr=getSetNr( setname );

    /* its a line */
    if(nr>-1)
    {
      div=atoi(type);
      //splitBiasDiv(&div, &bias);

      line[nr].div=div;
      //line[nr].bias=bias;
      repLine(nr);
    }
    /* its a set */
    else if(setNr>-1)
    {
      div=atoi(type);
      //splitBiasDiv(&div, &bias);
      for (j=0; j<set[setNr].anz_l; j++) if(line[set[setNr].line[j]].name!=NULL)
      {
        line[set[setNr].line[j]].div=div;
        //line[set[setNr].line[j]].bias=bias;
      }
      for (j=0; j<set[setNr].anz_l; j++) repLine(set[setNr].line[j]);
    }
    else
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
  }
  else
  {
    errMsg(" ERROR: command:|%s| not recognized\n", record );
  }
  return;
}


void pre_move( char *record)
{
  int  length, i,j,k,p, setNr, nr;
  char setname[MAX_LINE_LENGTH];
  char type[MAX_LINE_LENGTH];
  Nodes *bufn;
  char addDispFlagLocal=0;

  length = sword( record, setname );
  sword( &record[length], type );

  operateAlias( setname, "se" );
  setNr=getSetNr( setname );
  if (setNr<0)
  {
    errMsg(" ERROR: Set (%s) is undefined\n", setname );
    return;
  }
  delSet("-preMove");
  nr=pre_seta( "-preMove", "i", 0);
  for (i=0; i<set[setNr].anz_p; i++) seta( nr, "p", set[setNr].pnt[i]);
  for (i=0; i<set[setNr].anz_n; i++) seta( nr, "n", set[setNr].node[i]);
  // cycle through all shapes and add all points and nurbs
  for (i=0; i<set[setNr].anz_sh; i++)
  {
    k= set[setNr].shp[i];
    if(shape[k].type==0) for (j=0; j<3; j++) seta( nr, "p", shape[k].p[j] );
    else if(shape[k].type==1) for (j=0; j<3; j++) seta( nr, "p", shape[k].p[j] );
    else if(shape[k].type==2) for (j=0; j<4; j++) seta( nr, "p", shape[k].p[j] );
    else if(shape[k].type==3) for (j=0; j<7; j++) seta( nr, "p", shape[k].p[j] );
    else if(shape[k].type==4) seta( nr, "S", shape[k].p[0] );
    else if(shape[k].type==5) for (j=0; j<4; j++) seta( nr, "p", shape[k].p[j] );
  }
  /* cyrcle through all nurbs and add all points */
  for (i=0; i<set[setNr].anz_nurs; i++)
  {
    k= set[setNr].nurs[i];
    for (j=0; j<nurbs[k].u_npnt; j++)
      for (p=0; p<nurbs[k].v_npnt; p++)
        seta( nr, "p", nurbs[k].ctlpnt[j][p] );
  }

  if (set[nr].anz_p>set[nr].anz_n) i=set[nr].anz_p;
  else                             i=set[nr].anz_n;

  if ( (bufn = (Nodes *)malloc( (i+1) * sizeof(Nodes))) == NULL )
  {  printf("\n\n ERROR: malloc failed in pre_move(): i=%d\n\n", i) ; return; }


  if(set[nr].anz_p>0)
  {
    for (i=0; i<set[nr].anz_p; i++)
    {
      p=set[nr].pnt[i];
      if (p==-1)
      {
        errMsg(" ERROR: Point-nr:%d is undefined\n", set[nr].pnt[i] );
        return;
      }
      bufn[i].indx=p;
      bufn[i].nx=point[p].px*scale->w+scale->x;
      bufn[i].ny=point[p].py*scale->w+scale->y;
      bufn[i].nz=point[p].pz*scale->w+scale->z;
    }
    if(transform( &record[length], set[nr].anz_p, bufn )<0) return;

    for (i=0; i<set[nr].anz_p; i++)
    {
      p=set[nr].pnt[i];
      point[p].px= (bufn[i].nx-scale->x)/scale->w;
      point[p].py= (bufn[i].ny-scale->y)/scale->w;
      point[p].pz= (bufn[i].nz-scale->z)/scale->w;
    }
  }
  if(set[nr].anz_n>0)
  {
    /* when node coordinates were changed to the deformed ones then switch back before they are copied and then switch again */ 
    if(addDispFlag)
    {
      addDispToCoordinates(node);
      // remember to switch back
      addDispFlagLocal=2;
    }

    for (i=0; i<set[nr].anz_n; i++)
    {
      bufn[i].indx=set[nr].node[i];
      bufn[i].nx=node[set[nr].node[i]].nx* scale->w+scale->x;
      bufn[i].ny=node[set[nr].node[i]].ny* scale->w+scale->y;
      bufn[i].nz=node[set[nr].node[i]].nz* scale->w+scale->z;
    }
    if ( transform( &record[length], set[nr].anz_n, bufn) <0) return;
    for (i=0; i<set[nr].anz_n; i++)
    {
      node[set[nr].node[i]].nx= (bufn[i].nx-scale->x)/scale->w;
      node[set[nr].node[i]].ny= (bufn[i].ny-scale->y)/scale->w;
      node[set[nr].node[i]].nz= (bufn[i].nz-scale->z)/scale->w;
    }

    if(addDispFlagLocal==2)
    {
      addDispToCoordinates(node);
    }
  }

  /* the additional nodes for the HE20 etc. (then HE26) elements need a new pos. too  */
  if(anz->e>0)
  {
    // correct the elements, needed if the mesh was mirrored
    setall=getSetNr("all");
    elemChecker( set[setall].anz_e, set[setall].elem, node, e_enqire);
    adjustDrawNodes(0);
    getElemNormalen( e_enqire, node, anz->e );
    makeSurfaces();
    updateDispLists();
  }

  /* dependent higher entities need new label-positions, achived with orient */
  /* create a set with the dependent higher entities to be oriented  */

  /* add all points to special set _ORI  */
  if(set[nr].anz_p>0)
  {
    if( (j=pre_seta( specialset->ori, "i", 0 )) <0 ) return;
    for (k=0; k<set[nr].anz_p; k++)
    {
      p=set[nr].pnt[k];
      seta( j, "p", p );
    }
    completeSet( specialset->ori, "up");
    orientSet( specialset->ori );

    /*  recalculate the line-shapes    */
    for (i=0; i<set[j].anz_l; i++) repLine(set[j].line[i]);
    for (i=0; i<set[j].anz_nurl; i++) repNurl(set[j].nurl[i]);
    /*  recalculate the surface-shapes only if they are already filled   */
    nr=pre_seta("-filled","i",0);
    for (i=0; i<set[j].anz_s; i++) { if(surf[set[j].surf[i]].npgn) seta( nr, "s", set[j].surf[i] ); }
    completeSet( "-filled", "do");
    orientSet( "-filled" );
    for (i=0; i<set[nr].anz_nurs; i++) repNurs(set[nr].nurs[i]);
    for (i=0; i<set[nr].anz_nurs; i++) untrimNurs(set[nr].nurs[i]);
    pre_repSurf(nr);
    for (i=0; i<set[nr].anz_nurs; i++) untrimNurs(set[nr].nurs[i]);
    delSet("-filled");

    /* clear special set  */
    delSet(specialset->ori);
  }
  delSet("-preMove");
  free (bufn);
  if(( compare(type,"sca",3) == 3 )||( compare(type,"SCA",3) == 3 ))
  { frame(); frameSetFlag=-1; }
}



void pre_move_pfast( char *record)
{
  int  length, i,p, setNr, nr;
  char setname[MAX_LINE_LENGTH];
  char type[MAX_LINE_LENGTH];
  Nodes *bufn;

  length = sword( record, setname );
  sword( &record[length], type );

  setNr=getSetNr( setname );
  if (setNr<0)
  {
    errMsg(" ERROR: Set (%s) is undefined\n", setname );
    return;
  }
  nr=setNr;

  i=set[nr].anz_p;

  if ( (bufn = (Nodes *)malloc( (i+1) * sizeof(Nodes))) == NULL )
  {  printf("\n\n ERROR: malloc failed in pre_move(): i=%d\n\n", i) ; return; }


  if(set[nr].anz_p>0)
  {
    for (i=0; i<set[nr].anz_p; i++)
    {
      p=set[nr].pnt[i];
      if (p==-1)
      {
        errMsg(" ERROR: Point-nr:%d is undefined\n", set[nr].pnt[i] );
        return;
      }
      bufn[i].nx=point[p].px*scale->w+scale->x;
      bufn[i].ny=point[p].py*scale->w+scale->y;
      bufn[i].nz=point[p].pz*scale->w+scale->z;
    }
    if(transform( &record[length], set[nr].anz_p, bufn )<0) return;

    for (i=0; i<set[nr].anz_p; i++)
    {
      p=set[nr].pnt[i];
      point[p].px= (bufn[i].nx-scale->x)/scale->w;
      point[p].py= (bufn[i].ny-scale->y)/scale->w;
      point[p].pz= (bufn[i].nz-scale->z)/scale->w;
    }
  }
}



void free_proj(double *nx, double *nrad, int *nmissed)
{
  free (nx);
  free (nrad);
  free (nmissed);
  delSet(specialset->dep );
  delSet(specialset->ind );
}



void pre_proc( char *record)
{
  int  i;
  char action[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH];

  sscanf( record,"%s %s", action, param);

  for(i=0; i<strlen(action); i++) action[i]=toupper(action[i]);
  if (compareStrings(action, "DDIV")>0)
  {
    ddiv=atoi(param);
    splitBiasDiv(&ddiv, &dbias);
  }
}


/* l line, l_nr two new lines, ps_nr split-point */
int splitLine(int l, int *l_nr, int ps_nr)
{
  int  i,j,k,n,p,pp=0;
  int  setNr, setNr0, setNr1;
  char name[MAX_LINE_LENGTH], trk[2][MAX_LINE_LENGTH];

  int p1_nr, p2_nr, i_min;
  char linbuf[MAX_LINE_LENGTH];
  double lbez, lbez_min=MAX_FLOAT;
  double p0[3], p1[3], p01[3], v01[3], p10[3], v10[3];

  double ltot,l1,sp1,sp2,nodeSpacing;
  int    div1,div2, div, factor_div=1;

  if(printFlag) printf(" splitLine %s typ:%c trk:%d\n",line[l].name,line[l].typ,line[l].trk);
  
  /*  erzeuge den trk der neuen linien */
  if (line[l].typ=='a')
  {
    strcpy( trk[0], point[line[l].trk].name );
    strcpy( trk[1], point[line[l].trk].name );
  }
  else if (line[l].typ=='s')
  {    
    /* suche alle punkte des trk die vor dem schnittpunkt liegen und ordne sie einem */
    /* neuen set zu und loesche sie aus dem bestehenden trk */
    /* erzeuge zwei neue sets fuer die stuetzpunkte der zwei neuen linien */
    getNewName( trk[0], "se" );
    setNr0=pre_seta( trk[0], "is", 0);
  
    getNewName( trk[1], "se" );
    setNr1=pre_seta( trk[1], "is", 0);

    /*
     - search the intervall in which the split point exists.
       use v_scal to determine if the split point is in the interval
       
     - this algorith requires angles between successive line intervals greater as 90 deg
       increase div if necessary
    */
  increaseLineDiv:;
    repLine(l);  // scal might have changed
    setNr=line[l].trk;
    p0[0]=line[l].ip[0];
    p0[1]=line[l].ip[1];
    p0[2]=line[l].ip[2];
    i_min=-1;
    for (i=3; i<line[l].nip; i+=3)
    {
      lbez=0.;   
      p1[0]=line[l].ip[i]  ;
      p1[1]=line[l].ip[i+1];
      p1[2]=line[l].ip[i+2];
      v_result( p0, &point[ps_nr].px, p01  );
      lbez+=v_betrag( p01 );
      v_result( p1, &point[ps_nr].px, p01  );
      lbez+=v_betrag( p01 );
      if(lbez<lbez_min) { lbez_min=lbez; i_min=i; }
      //printf("lbez:%f lbez_min:%f i:%d i_min:%d\n", lbez, lbez_min, i,i_min );

      v_result( p0, p1, p01  );
      v_norm(p01, v01  );
      if(i>3)
      {
        sp1=v_sprod(v01,v10);
	if(sp1<0.)
	{
	  factor_div++;
	  line[l].div*=factor_div;
	  goto increaseLineDiv;
	}
      }
      v10[0]=v01[0];
      v10[1]=v01[1];
      v10[2]=v01[2];
      
      p0[0]=p1[0];
      p0[1]=p1[1];
      p0[2]=p1[2];
    }
    if(i_min==-1) { printf(" ERROR: Splitting of line:%s failed. No intersection found.\n", line[l].name); return(-1); }
    v_result(&line[l].ip[0], &line[l].ip[3], p01);
    nodeSpacing=v_betrag( p01 );
    //printf(" splitpnt:%s xyz %f %f %f nodeSpacing:%f i_min:%d\n",point[ps_nr].name,point[ps_nr].px,point[ps_nr].py,point[ps_nr].pz, nodeSpacing, i_min );
  
    // use v_scal to determine if the current point is in the interval, then take next point
    k=0;
    seta( setNr0, "ps", line[l].p1 );
    if( line[l].p1==set[setNr].pnt[0] ) pp=1;
    else { printf(" ERROR in seq:%s\n", set[setNr].name); return(-1); }
    //p=0; printf("l1 p[%d]:%s p %f %f %f \n",p, point[set[setNr].pnt[p]].name, point[set[setNr].pnt[p]].px,point[set[setNr].pnt[p]].py,point[set[setNr].pnt[p]].pz);
    p0[0]=line[l].ip[0];
    p0[1]=line[l].ip[1];
    p0[2]=line[l].ip[2];
    for (i=3; i<line[l].nip; i+=3)
    {
      p1[0]=line[l].ip[i]  ;
      p1[1]=line[l].ip[i+1];
      p1[2]=line[l].ip[i+2];
      v_result( p0, p1, v01  );
      v_result( p1, p0, v10  );
      v_norm(v01, v01  );
      v_norm(v10, v10  );
      //printf("i:%d line[l].nip:%d p1 %f %f %f p2: %f %f %f\n",i,line[l].nip,p0[0],p0[1],p0[2],p1[0],p1[1],p1[2]);
      //printf(" v01 %f %f %f v10: %f %f %f\n",v01[0],v01[1],v01[2],v10[0],v10[1],v10[2]);
      if(i<=i_min)
      {
        //printf("loop1 pp:%d from set[setNr].anz_p:%d\n",pp,set[setNr].anz_p);
        // search all points in the interval and add them to the set
        for ( ; pp<set[setNr].anz_p; pp++)
        {
	  if( line[l].p1==set[setNr].pnt[0] ) p=pp;
          else p=set[setNr].anz_p-1-pp;

          //printf("l1 pp:%d p[%d]:%s pxyz %f %f %f \n",pp, p, point[set[setNr].pnt[p]].name, point[set[setNr].pnt[p]].px,point[set[setNr].pnt[p]].py,point[set[setNr].pnt[p]].pz);
	  
          // use v_scal to determine if the current point is in the interval, then take next point (TBD)
	  // the scalar product must be twice positive
          v_result( p0, &point[set[setNr].pnt[p]].px, p01  );
          v_result( p1, &point[set[setNr].pnt[p]].px, p10  );
          l1=v_norm(p01, p01  );
          if(l1) sp1=v_sprod(p01,v01); else sp1=0.;
          l1=v_norm(p10, p10  );
          if(l1) sp2=v_sprod(p10,v10); else sp2=0.;
          //printf("i:%d line[l].nip:%d p1 %f %f %f p2: %f %f %f\n",i,line[l].nip,p0[0],p0[1],p0[2],p1[0],p1[1],p1[2]);
	  //printf(" p01 %f %f %f p10: %f %f %f \n",p01[0],p01[1],p01[2],p10[0],p10[1],p10[2]);
	  //printf("  sp %e %e \n",sp1,sp2);
          if((sp1>=0.)&&(sp2>=0.))
	  {
    	    if(i==i_min)
	    {
	      // skip if the point is behind the splitpoint
              v_result( &point[ps_nr].px, &point[set[setNr].pnt[p]].px, p10  );
              l1=v_norm(p10, p10  );
              if(l1) sp2=v_sprod(p10,v10); else sp2=0.;
	      if((sp1>=0.)&&(sp2>=0.))
	      {
                /* use the last track-point only if the distance to ps_nr is significant */
                v_result( &point[set[setNr].pnt[p]].px, &point[ps_nr].px, p01  );
  	        if(v_betrag( p01 )>nodeSpacing*.1) seta( setNr0, "ps", set[setNr].pnt[p] );
	        //else printf(" skip p:%s, distance to ps_nr is too close\n",point[set[setNr].pnt[p]].name);
	      }
	      else
	      {
		//printf(" skip p:%s, point is behind the splitpoint\n",point[set[setNr].pnt[p]].name);

	        // block is a copy from below
        if(!k)
        {
          //printf("switch at pp:%d p[%d]:%s\n",pp, p, point[set[setNr].pnt[p]].name);

	  // go over the points up to the splitpoint
          for (; pp<set[setNr].anz_p; pp++)
          {
  	    if( line[l].p1==set[setNr].pnt[0] ) p=pp;
            else p=set[setNr].anz_p-1-pp;
  	  
  	    // the scalar product in the interval to ps_nr  must be twice positive
            v_result( p0, &point[set[setNr].pnt[p]].px, p01  );
            v_result( &point[ps_nr].px, &point[set[setNr].pnt[p]].px, p10  );
            l1=v_norm(p01, p01  );
            if(l1) sp1=v_sprod(p01,v01); else sp1=0.;
            l1=v_norm(p10, p10  );
            if(l1) sp2=v_sprod(p10,v10); else sp2=0.;
            //printf("l2.1 p[%d]:%s  sp %f %f\n",p, point[set[setNr].pnt[p]].name, sp1,sp2);
  	    if((sp1>=0.)&&(sp2>=0.))
  	    {
              /* use the last track-point only if the distance to ps_nr is significant */
              v_result( &point[set[setNr].pnt[p]].px, &point[ps_nr].px, p01  );
  	      if(v_betrag( p01 )>nodeSpacing*.1) seta( setNr0, "ps", set[setNr].pnt[p] );
	      //else printf(" skip p:%s\n",point[set[setNr].pnt[p]].name);
   	    }
            else break;
          }
	
          seta( setNr0, "ps", ps_nr ); 
          seta( setNr1, "ps", ps_nr ); k++;
        }
	// end copied block
	seta( setNr1, "ps", set[setNr].pnt[p] );
	      }
	  
	    }
	    else  seta( setNr0, "ps", set[setNr].pnt[p] );
   	  }
          else
	  {
	    //printf(" outside p:%s\n",point[set[setNr].pnt[p]].name);
	    break;
	  }
        }
      }
      else
      {
	// following block copied and used also above!
        if(!k)
        {
          //printf("switch at pp:%d p[%d]:%s\n",pp, p, point[set[setNr].pnt[p]].name);

	  // go over the points up to the splitpoint
          for (; pp<set[setNr].anz_p; pp++)
          {
  	    if( line[l].p1==set[setNr].pnt[0] ) p=pp;
            else p=set[setNr].anz_p-1-pp;
  	  
  	    // the scalar product in the interval to ps_nr  must be twice positive
            v_result( p0, &point[set[setNr].pnt[p]].px, p01  );
            v_result( &point[ps_nr].px, &point[set[setNr].pnt[p]].px, p10  );
            l1=v_norm(p01, p01  );
            if(l1) sp1=v_sprod(p01,v01); else sp1=0.;
            l1=v_norm(p10, p10  );
            if(l1) sp2=v_sprod(p10,v10); else sp2=0.;
            //printf("l2.1 p[%d]:%s  sp %f %f\n",p, point[set[setNr].pnt[p]].name, sp1,sp2);
  	    if((sp1>=0.)&&(sp2>=0.))
  	    {
              /* use the last track-point only if the distance to ps_nr is significant */
              v_result( &point[set[setNr].pnt[p]].px, &point[ps_nr].px, p01  );
  	      if(v_betrag( p01 )>nodeSpacing*.1) seta( setNr0, "ps", set[setNr].pnt[p] );
	      //else printf(" skip p:%s\n",point[set[setNr].pnt[p]].name);
   	    }
            else break;
          }
	
          seta( setNr0, "ps", ps_nr ); 
          seta( setNr1, "ps", ps_nr ); k++;
        }
	// end copied block
	
        // search all points in the interval and add them to the set
        //printf("loop2 pp:%d from set[setNr].anz_p:%d\n",pp,set[setNr].anz_p);
        for (;pp<set[setNr].anz_p; pp++)
        {
	  if( line[l].p1==set[setNr].pnt[0] ) p=pp;
          else p=set[setNr].anz_p-1-pp;
	  
	  // the scalar product must be twice positive
          v_result( p0, &point[set[setNr].pnt[p]].px, p01  );
          v_result( p1, &point[set[setNr].pnt[p]].px, p10  );
          l1=v_norm(p01, p01  );
          if(l1) sp1=v_sprod(p01,v01); else sp1=0.;
          l1=v_norm(p10, p10  );
          if(l1) sp2=v_sprod(p10,v10); else sp2=0.;
          //printf("l2.2 p[%d]:%s  sp %f %f set:%s\n",p, point[set[setNr].pnt[p]].name, sp1,sp2, set[setNr1].name);
  	  if((sp1>=0.)&&(sp2>=0.))
  	  {
            if(k==1) // only in the second part of the switching interval
	    {
              /* use the last track-point only if the distance to ps_nr is significant */
              v_result( &point[set[setNr].pnt[p]].px, &point[ps_nr].px, p01  );
  	      if(v_betrag( p01 )>nodeSpacing*.1) seta( setNr1, "ps", set[setNr].pnt[p] );
	      //else printf("v_betrag( p01 ):%e  nodeSpacing:%e skip p:%s\n",v_betrag( p01 ),nodeSpacing,point[set[setNr].pnt[p]].name);
	    }
	    else seta( setNr1, "ps", set[setNr].pnt[p] );
   	  }
          else break;
        }
      }
      p0[0]=p1[0];
      p0[1]=p1[1];
      p0[2]=p1[2];
    }
    if(set[setNr1].anz_p==0) { printf(" ERROR: line %s not splitted\n", line[l].name); return(-1); }
    if(line[l].p2!=set[setNr1].pnt[set[setNr1].anz_p-1])
      seta( setNr1, "ps", line[l].p2 );
    //printf("p2:%s plast:%s\n", point[line[l].p2].name, point[set[setNr1].pnt[set[setNr1].anz_p-1]].name);

    // must stay spline because of aflib
    //if(set[setNr0].anz_p<=2) strcpy(trk[0], " ");
    if(set[setNr0].anz_p<2) strcpy(trk[0], " ");
    else
    {
      // check the trk regarding the track-point distances to the line endpoints
      // prevent situations were the closest distance of any track point is nearly the line length
      lbez_min=MAX_FLOAT;
      for(i=1; i<set[setNr0].anz_p-1; i++)
      {
        v_result( &point[set[setNr0].pnt[i]].px, &point[set[setNr0].pnt[0]].px, p0  );
        lbez=v_betrag(p0);
        if(lbez<lbez_min) { lbez_min=lbez; i_min=i; }
        //printf("1lbez:%f lbez_min:%f i:%d i_min:%d\n", lbez, lbez_min, i,i_min );
      }
      sp1=lbez_min;
      lbez_min=MAX_FLOAT;
      for(i=1; i<set[setNr0].anz_p-1; i++)
      {
        v_result( &point[set[setNr0].pnt[i]].px, &point[set[setNr0].pnt[set[setNr0].anz_p-1]].px, p0  );
        lbez=v_betrag(p0);
        if(lbez<lbez_min) { lbez_min=lbez; i_min=i; }
        //printf("2lbez:%f lbez_min:%f i:%d i_min:%d\n", lbez, lbez_min, i,i_min );
      }
      sp2=lbez_min;
      v_result( &point[set[setNr0].pnt[0]].px, &point[set[setNr0].pnt[set[setNr0].anz_p-1]].px, p0  );
      lbez=v_betrag(p0);
      //printf("lbez:%f \n", lbez );
      if((lbez*0.95<sp1)||(lbez*0.95<sp2))
      {
	// must stay spline because of aflib
	//strcpy(trk[0], " ");
	set[setNr0].pnt[1]=set[setNr0].pnt[set[setNr0].anz_p-1];
	set[setNr0].anz_p=2;
      }
    }
    
    // must stay spline because of aflib
    //if(set[setNr1].anz_p<=2) strcpy(trk[1], " ");
    if(set[setNr1].anz_p<2) strcpy(trk[1], " ");
    else
    {
      // check the trk regarding the track-point distances to the line endpoints
      // prevent situations were the closest distance of any track point is nearly the line length
      lbez_min=MAX_FLOAT;
      for(i=1; i<set[setNr1].anz_p-1; i++)
      {
        v_result( &point[set[setNr1].pnt[i]].px, &point[set[setNr1].pnt[0]].px, p0  );
        lbez=v_betrag(p0);
        if(lbez<lbez_min) { lbez_min=lbez; i_min=i; }
        //printf("1lbez:%f lbez_min:%f i:%d i_min:%d\n", lbez, lbez_min, i,i_min );
      }
      sp1=lbez_min;
      lbez_min=MAX_FLOAT;
      for(i=1; i<set[setNr1].anz_p-1; i++)
      {
        v_result( &point[set[setNr1].pnt[i]].px, &point[set[setNr1].pnt[set[setNr1].anz_p-1]].px, p0  );
        lbez=v_betrag(p0);
        if(lbez<lbez_min) { lbez_min=lbez; i_min=i; }
        //printf("2lbez:%f lbez_min:%f i:%d i_min:%d\n", lbez, lbez_min, i,i_min );
      }
      sp2=lbez_min;
      v_result( &point[set[setNr1].pnt[0]].px, &point[set[setNr1].pnt[set[setNr1].anz_p-1]].px, p0  );
      lbez=v_betrag(p0);
      //printf("lbez:%f \n", lbez );
      if((lbez*0.95<sp1)||(lbez*0.95<sp2))
      {
	// must stay spline because of aflib
	//strcpy(trk[1], " ");
	set[setNr1].pnt[1]=set[setNr1].pnt[set[setNr1].anz_p-1];
	set[setNr1].anz_p=2;
      }
    }
  }
  else
  {
    strcpy(trk[0], " ");
    strcpy(trk[1], " ");
  }

  /* sichere den liniennamen bevor der pointer durch realloc in line_ geaendert wird  */
  strcpy( linbuf, line[l].name);

  /* erzeuge zwei neue linien  */
  p1_nr = line[l].p1;
  p2_nr = line[l].p2;
  div=1;
  getNewName( name, "l" );
  // Warning: aflib needs the line track as set of points, even if only 2 points
  // change the code further above if the problem is solved on the aflib side (search pre_split there)
  if(printFlag) printf(" create line:%s %s %s %s %d\n", name, point[p1_nr].name, point[ps_nr].name, trk[0], div );
  line_( name, point[p1_nr].name, point[ps_nr].name, trk[0], div, 1 );
  l_nr[0]=getLineNr(name);
  getNewName( name, "l" );
  if(printFlag) printf(" create line:%s %s %s %s %d\n", name, point[ps_nr].name, point[p2_nr].name, trk[1], div );
  line_( name, point[ps_nr].name, point[p2_nr].name, trk[1], div, 1 );
  l_nr[1]=getLineNr(name);

  /* add snew to sets which contain linbuf */
  for(i=0; i<anz->sets; i++)
  {
    if(!set[i].type)
      if((set[i].name!=(char *)NULL)&&(set[i].name[0]!='-')&&(i!=setall)&&( getIndex(&set[i].line,set[i].anz_l,l) >-1)) { seta(i,"l",l_nr[0]); seta(i,"l",l_nr[1]); }
  }

  /* calc new div */
  line[l].div/=factor_div;
  if (line[l].div>1)
  {
    ltot=calcLineLength(l);
    l1=calcLineLength(l_nr[0]);
    div1=line[l].div/ltot*l1;
    // force an even number
    if(div1%2!=0) div1++;
    div2=line[l].div-div1;
    if(printFlag)  printf("1 ltot:%lf l1:%lf ldiv:%d div1:%d div2:%d\n", ltot,l1,line[l].div,div1,div2); 

    if(div1<2) { div1=2; div2=line[l].div-div1; }
    if(div2<2) { div2=2; }
    line[l_nr[0]].div=div1;
    line[l_nr[1]].div=div2;
    if(printFlag)  printf("2 ltot:%lf l1:%lf ldiv:%d div1:%d div2:%d\n", ltot,l1,line[l].div,div1,div2); 
    repLine(l_nr[0]);
    repLine(l_nr[1]);
  }
  /* untersuche alle lcmbs ob linbuf ein Mitglied ist */
  for (i=0; i<anzGeo->c; i++) if( lcmb[i].name != (char *)NULL )
  {
    for (j=0; j<lcmb[i].nl; j++)
    {
      if( l == lcmb[i].l[j] )
      {
        //printf (" realloc lcmb:%s and replace line:%s with %s and %s \n",  lcmb[i].name, line[l].name,line[l_nr[0]].name, line[l_nr[1]].name );
        if ((lcmb[i].o = (char *)realloc( (char *)lcmb[i].o, (lcmb[i].nl+1)*sizeof(char)) ) == NULL )
        { printf("\n\n ERROR: realloc failure in qspl, lcmb.o:%s not changed\n\n",lcmb[i].name ); return(-1); }
        if ((lcmb[i].l = (int *)realloc( (int *)lcmb[i].l, (lcmb[i].nl+1)*sizeof(int)) ) == NULL )
        { printf("\n\n ERROR: realloc failure in qspl, lcmb.l:%s not changed\n\n", lcmb[i].name); return(-1); }
	/* umspeichern der linien beginnend bei der letzten bis einschlieslich j */
        for (n=lcmb[i].nl; n>j; n--)
	{
          lcmb[i].o[n]=lcmb[i].o[n-1];
          lcmb[i].l[n]=lcmb[i].l[n-1];
        }
        /* Auffuellen der j, j+1 pos. mit l1, l2 mit der Orientierung der gesplitteten linie */
        lcmb[i].o[j+1]=lcmb[i].o[j];
        if(lcmb[i].o[j]=='+')
	{
          lcmb[i].l[j]=l_nr[0];
          lcmb[i].l[j+1]=l_nr[1];
	}
        else
	{
          lcmb[i].l[j+1]=l_nr[0];
          lcmb[i].l[j]=l_nr[1];
	}
        lcmb[i].nl++;
      }
    }
  }


  /* untersuche alle surfs ob linbuf ein Mitglied ist und ersetze sie durch eine lcmb */
  /* kontrolliere ob nicht schon eine geeignete lcmb existiert */
  for (i=0; i<anzGeo->s; i++) if( surf[i].name != (char *)NULL )
  {
    for (j=0; j<surf[i].nl; j++)
    {
      if(( l == surf[i].l[j] )&&( surf[i].typ[j]=='l' ))
      {

        /* do we already have a suitable lcmb? */
        for (n=0; n<anzGeo->c; n++ ) if( lcmb[n].name != (char *)NULL )
	{
          if (lcmb[n].nl==2)   /* same amount of lines */
	  {
	    /*
            printf ("checke lcmb:%s \n", lcmb[n].name);
	    */
            if (((lcmb[n].l[0]==l_nr[0])||(lcmb[n].l[0]==l_nr[1])) && ((lcmb[n].l[1]==l_nr[0])||(lcmb[n].l[1]==l_nr[1])))
	    {
	      /*
              printf ("equal:%s\n",lcmb[n].name);
	      */
              break;
	    }
	  }
        }
        if (n>=anzGeo->c)  /* no lcmb was found, so create one */
        {
          /* create lcmb */
          if ( getNewName( name, "c" ) == -1 )
          { printf("Type c not known, lcmb can not be created\n"); exit(-1); }
          lcmb_i( name, (int)0, (int)2, "++", l_nr );
          n=getLcmbNr( name );
        }
        //printf ("realloc surf:%s and replace line:%s with lcmb:%s made of %s and %s \n", surf[i].name, line[l].name, name, line[l_nr[0]].name, line[l_nr[1]].name );
        if (n>-1) { surf[i].l[j]=n; surf[i].typ[j]='c'; }
        else { errMsg("lcmb not known, surface could not be changed \n"); return(-1); }
      }
    }
  }

  return(1);
}


int genSplitTrias(int trgtNr, Tri **ptr_tri, int scalFlag )
{
  int i,j,n,s;
  int sum_tri=0, ss;
  Tri *tri;

  tri=*ptr_tri;

  for (ss=0; ss<set[trgtNr].anz_sh; ss++)
  {
    s=set[trgtNr].shp[ss];
    if(printFlag) printf("shape:%s\n", shape[s].name);

    /* go over all shapes */
    n=0;
    while((shape[s].npgn-n))
    {
      /* alloc a new tri */
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+2) * sizeof(Tri))) == NULL )
      {
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      }

      /* requires that only triangles are used for the interiour description of the shape */
      /* this is regarded by the adjustFeedBack() routine */
      //n+=5; /* jump over the leading bytes */
      tri[sum_tri].p1[0]=shape[s].pgn[n++];
      tri[sum_tri].p1[1]=shape[s].pgn[n++];
      tri[sum_tri].p1[2]=shape[s].pgn[n++];
      tri[sum_tri].p2[0]=shape[s].pgn[n++];
      tri[sum_tri].p2[1]=shape[s].pgn[n++];
      tri[sum_tri].p2[2]=shape[s].pgn[n++];
      tri[sum_tri].p3[0]=shape[s].pgn[n++];
      tri[sum_tri].p3[1]=shape[s].pgn[n++];
      tri[sum_tri].p3[2]=shape[s].pgn[n++];
      tri[sum_tri].fnr= -1;
      sum_tri++;
    }
  }
  /* go over all surfs */
  for (ss=0; ss<set[trgtNr].anz_s; ss++)
  {
    s=set[trgtNr].surf[ss];
    if(printFlag) printf("surface:%s\n", surf[s].name);

    /* go over all tri */
    n=0;
    while((surf[s].npgn-n))
    {
      /* alloc a new tri */
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+2) * sizeof(Tri))) == NULL )
      {
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      }

      /* requires that only triangles are used for the interiour description of the surface */
      /* this is regarded by the adjustFeedBack() routine */
      n+=5; /* jump over the leading bytes */
      tri[sum_tri].p1[0]=surf[s].pgn[n++];
      tri[sum_tri].p1[1]=surf[s].pgn[n++];
      tri[sum_tri].p1[2]=surf[s].pgn[n++];
      tri[sum_tri].p2[0]=surf[s].pgn[n++];
      tri[sum_tri].p2[1]=surf[s].pgn[n++];
      tri[sum_tri].p2[2]=surf[s].pgn[n++];
      tri[sum_tri].p3[0]=surf[s].pgn[n++];
      tri[sum_tri].p3[1]=surf[s].pgn[n++];
      tri[sum_tri].p3[2]=surf[s].pgn[n++];
      tri[sum_tri].fnr= -1;
      sum_tri++;
    }
  }
  for (ss=0; ss<set[trgtNr].anz_f; ss++)
  {
    i=set[trgtNr].face[ss];

    if (face[i].type == 7)  /* TRI3  */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+2) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].fnr= i;
      sum_tri++;
    }
    else if (face[i].type == 8)  /* TRI6  */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+6) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[5]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[5]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[4]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[4]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[5]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[3]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[4]].nz;
      sum_tri++;
    }
    else if (face[i].type == 9)  /* QUAD4 */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+4) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].fnr= i;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].fnr= i;
      sum_tri++;
    }
    else if (face[i].type == 10) /* QUAD8 */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+8) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[4]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[4]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[1]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[5]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[5]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[2]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[6]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[6]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[6]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[6]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[6]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[6]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[3]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[7]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[7]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[7]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[7]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[7]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[7]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[0]].nz;
      sum_tri++;
    }
  }
  for(j=0; j<sum_tri; j++)
  {
    if(scalFlag)
    {
      tri[j].p1[0]= tri[j].p1[0]*scale->w+scale->x; tri[j].p1[1]=tri[j].p1[1]* scale->w+scale->y; tri[j].p1[2]= tri[j].p1[2]*scale->w+scale->z;
      tri[j].p2[0]= tri[j].p2[0]*scale->w+scale->x; tri[j].p2[1]=tri[j].p2[1]* scale->w+scale->y; tri[j].p2[2]= tri[j].p2[2]*scale->w+scale->z;
      tri[j].p3[0]= tri[j].p3[0]*scale->w+scale->x; tri[j].p3[1]=tri[j].p3[1]* scale->w+scale->y; tri[j].p3[2]= tri[j].p3[2]*scale->w+scale->z;
    }

    /* "cg" of tri       */
    tri[j].cg[0]=(tri[j].p1[0]+tri[j].p2[0]+tri[j].p3[0])/3;
    tri[j].cg[1]=(tri[j].p1[1]+tri[j].p2[1]+tri[j].p3[1])/3;
    tri[j].cg[2]=(tri[j].p1[2]+tri[j].p2[2]+tri[j].p3[2])/3;
  }

  *ptr_tri=tri;
  return(sum_tri);
}


/* setSplit: lines which were generated by the previous surface splitting
   setSurf:  surfaces which were the result of the previous splitting
*/
void separateSurfs(int setSplit, int setSurf)
{
  int i,j,k,l,ll,m,p,s,c, l1,l2,s1,s2;
  int set1, set2, buf1, buf2, settrgt, setbuf, *surfused=NULL;
  int p1,p2,p3,p4,p5,p6,p7,se,nr,b,S,anz_s_buf=0;
  int *dep_p=NULL, *dep_l=NULL, *dep_sh=NULL, *dep_S=NULL;
  char name[MAX_LINE_LENGTH];
  char setname[MAX_LINE_LENGTH];
  
  printf("\n split set:%s with lines:%s\n\n", set[setSurf].name, set[setSplit].name);
  if(set[setSurf].anz_s<1) return;
  if(set[setSplit].anz_l<1) return;

  delSet( "-buf1" );
  delSet( "-buf2" );
  delSet( "+set1" );
  delSet( "+set2" );
  delSet( "+cut2" );
  set1=pre_seta("+set1","i",0);   // all connected surfaces on one side of the cut
  set2=pre_seta("+set2","i",0);   // all connected surfaces on the other side of the cut
  buf1=pre_seta("-buf1","i",0);   
  buf2=pre_seta("-buf2","i",0);   
  settrgt=pre_seta("+cut2","i",0);

  delSet( "-setbuf" );
  setbuf=pre_seta("-setbuf","i",0);

  // separate the surfs into two banks (as long as sum_surfs +buf1 + +buf2 < setSplit)
  //
  // place the first surf in buf1
  // take the first surf from buf1, check its outer lines
  // - if a line from setSplit is found search the neighbor surf and place it in buf2
  // - else search the neighbor surf and place it in buf1
  // take the next surf from buf1, check its outer lines etc.
  // check if sum_surfs +buf1 + +buf2 has increased
  // - if not: parts of the splitted geometry are disjunct, add an unused surf to -buf1
  //   but before store all surfs from buf1 and buf2  into set1 and set2
  //   these will later be used to generate the final set1 and set2

  //printf("- separate the surfs into two banks\n");

  if( (surfused = (int *)calloc((set[setSurf].anz_s+1),sizeof(int) ))==NULL )
    printf("ERROR: calloc failure\n");
  
  seta(set1, "s", set[setSurf].surf[0]);
  seta(buf1, "s", set[setSurf].surf[0]); 

  do
  {
    for(i=0; i<set[buf1].anz_s; i++)
    {
      s1=set[buf1].surf[i];
      //printf("do i:%d from %d , check surf:%s from +buf1 containing %d surfs\n",i,set[buf1].anz_s,surf[s1].name);
      for(j=0; j<surf[s1].nl; j++)
      {
        l1=surf[s1].l[j];
        if(surf[s1].typ[j]=='l')  // the connecting edge between +buf1 and +buf2 is always a line!
        {
          //printf(" %d check line %s against %d surfs from setSurf, check only unasigned\n",j, line[l1].name, set[setSurf].anz_s);
          for(k=0; k<set[setSurf].anz_s; k++)
          {
            if(surfused[k]==1) continue;
            s2=set[setSurf].surf[k];
            //printf("  k:%d compare surf[%d]:%s\n",k,s2,surf[s2].name);
            for(l=0; l<surf[s2].nl; l++)
            {
              l2=surf[s2].l[l];
              if((surf[s2].typ[l]=='l')&&(l1==l2))
              {
                //printf("   %s matches %s, matches setSplit as well?\n",line[l1].name,line[l2].name);
                if( getIndex(&set[setSplit].line,set[setSplit].anz_l,l1) >-1)
                {
                  //printf("   %s matches splitline, store surf in s2\n",line[l1].name);
                  seta(buf2, "s", s2); 
                  if(!anz_s_buf) seta(set2, "s", s2); 
                }
	        else
		{
		  seta(buf1, "s", s2);
                  if(!anz_s_buf) seta(set1, "s", s2); 
                }
                surfused[k]=1;
              }
            } 
          }
        }
      }
    }
    //printf("set[buf1].anz_s:%d set[buf2].anz_s:%d anz_s_buf:%d set[setSurf].anz_s:%d\n",set[buf1].anz_s,set[buf2].anz_s,anz_s_buf,set[setSurf].anz_s);
    if((set[buf1].anz_s+set[buf2].anz_s)==anz_s_buf)
    {
      // parts of the splitted geometry are disjunct, add an unused surf to +buf1
      for(k=0; k<set[setSurf].anz_s; k++)
      {
        if(surfused[k]==1) continue;
        seta(buf1, "s", set[setSurf].surf[k]);
        //printf(" parts of the splitted geometry are disjunct, add an unused surf to +buf1:%s\n",surf[set[setSurf].surf[k]].name);
        break;
      }
    }
    anz_s_buf=set[buf1].anz_s+set[buf2].anz_s;
  }while((set[buf1].anz_s+set[buf2].anz_s) < set[setSurf].anz_s);
  free(surfused);
  
  // replace the setSplit lines with copies in buf2
  // copy set[setSplit].line with needed entities, copies must reference origins (see copySet())
  // replace copied lines in buf2

  //printf("- copy points\n");

  completeSet(set[setSplit].name, "do");
  
  /* copy points */
  if(set[setSplit].anz_p>0)
  {
    if ( (dep_p = (int *)realloc((int *)dep_p, (anzGeo->p+1) * sizeof(int))) == NULL )
    { printf(" ERROR: realloc failed\n"); return; }
    for (i=0; i<anzGeo->p; i++) dep_p[i]=-1;
    for (i=0; i<set[setSplit].anz_p; i++)
    {
      p= getNewName( name, "p" );
      if ( p == -1 )
      { printf(" ERROR: could not create new point\n"); return; }
      p=set[setSplit].pnt[i];
      p=pnt( name, point[p].px,point[p].py,point[p].pz, 0 );
      if ( p <0 )
      { printf(" ERROR: could not create new point\n"); return; }
      dep_p[set[setSplit].pnt[i]] = p; /* remember the index of the new point */
      //seta( settrgt, "p", p );
    }
  }

  //printf("- copy lines\n");
  
  /* copy the lines */
  if(set[setSplit].anz_l>0)
  {
    if ( (dep_l = (int *)realloc((int *)dep_l, (anzGeo->l+1) * sizeof(int))) == NULL )
    { printf(" ERROR: realloc failed\n"); return; }
    j=set[setSplit].anz_l;
    for (i=0; i<j; i++)
    {
      l=set[setSplit].line[i];
      if (l==-1)
      {
        errMsg(" Linenr:%d is undefined\n", set[setSplit].line[i] );
        return;
      }
      l= getNewName( name, "l" );
      if ( l == -1 )
      { printf(" ERROR: could not create new line\n"); return; }
      p1=line[set[setSplit].line[i]].p1;
      p2=line[set[setSplit].line[i]].p2;
      p3=line[set[setSplit].line[i]].trk;
      if (line[set[setSplit].line[i]].typ==' ')     
        l=line_i( name, dep_p[p1], dep_p[p2], 0, line[set[setSplit].line[i]].div, line[set[setSplit].line[i]].bias, 0 ); 
      else if (line[set[setSplit].line[i]].typ=='a') 
        l=line_i( name, dep_p[p1], dep_p[p2], dep_p[p3], line[set[setSplit].line[i]].div, line[set[setSplit].line[i]].bias, 'a' );
      else if (line[set[setSplit].line[i]].typ=='s')
      {
        /* copy set */
        se= getNewName( setname, "se" );
        if ( se == -1 )
          { printf(" ERROR: could not create new set\n"); return; }
        if(set[p3].type==1) se=pre_seta( setname, "is", 0 );
        else se=pre_seta( setname, "i", 0 );
        if ( se <0 )
          { printf("copy: could not create new set\n"); return; }
  
        for (k=0; k<set[p3].anz_p; k++)
        {
          p=dep_p[set[p3].pnt[k]];
          if( getPntNr(point[p].name)<0 )
          { printf(" ERROR: could not find dep_p:%s from p:%s (from set:%s)\n", point[p].name, point[set[p3].pnt[k]].name, set[p3].name ); return; }
          seta( se, "p", p );
        }
        //dep_se[p3] = se; /* remember the index of the new set */
        //seta( settrgt, "r", se );

        l=line_i( name, dep_p[p1], dep_p[p2], se, line[set[setSplit].line[i]].div, line[set[setSplit].line[i]].bias, 's' );
      }
      if ( l <0 )
      { printf("copy: could not create new line\n"); return; }
      line[l].etyp =line[set[setSplit].line[i]].etyp;
      line[l].eattr=line[set[setSplit].line[i]].eattr;
      dep_l[set[setSplit].line[i]] = l; /* remember the index of the new line */
      seta( settrgt, "l", l );

      /* add to all sets were the orig is a member */
      for(se=1; se<anz->sets; se++) if((se!=settrgt)&&(se!=setSplit)&&(set[se].name!=(char *)NULL)&&(set[se].name[0]!='-')) if(!set[se].type) if( getIndex(&set[se].line, set[se].anz_l, set[setSplit].line[i]) >-1) { seta( se, "l", l );  }
    }
  }

  //printf("- replace the setSplit lines and points, modify splines with copies in buf2\n");

  /* replace the copied lines and points in the surfaces of buf2 */
  for(i=0; i<set[buf2].anz_s; i++)
  {
    s2=set[buf2].surf[i];
    //printf("%d mod surf:%s\n",i,surf[s2].name);
    for(j=0; j<surf[s2].nl; j++)
    {
      l2=surf[s2].l[j];
      if(surf[s2].typ[j]=='l')
      {
        //printf(" %d check line %s\n",j, line[l2].name);
        k=getIndex(&set[setSplit].line,set[setSplit].anz_l,l2);
        if(k>-1)
        {
          //printf(" %s matches splitline\n",line[set[setSplit].line[k]].name);
          surf[s2].l[j]=dep_l[l2];
          l2=surf[s2].l[j];
        }
        else
	{
          // change points
          //printf(" change points in line %s ltyp %c\n",line[l2].name, line[l2].typ);
          p1=p2=-1;
          if( dep_p[line[l2].p1]!=-1) { p1=line[l2].p1; line[l2].p1=dep_p[line[l2].p1]; }
          if( dep_p[line[l2].p2]!=-1) { p2=line[l2].p2; line[l2].p2=dep_p[line[l2].p2]; }
	  if (line[l2].typ=='a') { if( dep_p[line[l2].trk]!=-1) line[l2].trk=dep_p[line[l2].trk]; }
          else if (line[l2].typ=='s')
          {
            se=line[l2].trk;
            if(p1>-1)
            {
              //printf(" line.p1 %s was updated, update trk %s\n",point[p1].name, set[se].name);
              for(k=0; k<set[se].anz_p; k++)
              {
                if(set[se].pnt[k]==p1)
                { set[se].pnt[k]=line[l2].p1;
		  //printf(" trkp:%d %s\n",k,point[line[l2].p1].name);
		  break; }
              }
            }
            if(p2>-1)
            {
              //printf(" line.p2 %s was updated, update trk %s\n",point[p2].name, set[se].name);
              for(k=0; k<set[se].anz_p; k++)
              {
                if(set[se].pnt[k]==p2)
                { set[se].pnt[k]=line[l2].p2;
		  //printf(" trkp:%d %s\n",k,point[line[l2].p2].name);
		  break;
		}
              }
            }
	  }
	  // search lcmb which references the line, check if this lcmb is used in a surf, del if not
	  if((p1>-1)||(p2>-1))
	  {
            for(k=0; k<anzGeo->c; k++)
            {
	      if(lcmb[k].name!=NULL) for(c=0; c<lcmb[k].nl; c++)
	      {
                if(lcmb[k].l[c]==l2)
	  	{
	  	  m=1;
                  for(s=0; s<set[buf2].anz_s; s++)
                  {
                    for( p=0; p<surf[set[buf2].surf[s]].nl; p++)
	            {
		      if((surf[set[buf2].surf[s]].typ[p]!='l')&&(surf[set[buf2].surf[s]].l[p]==k)) m=0;
	  	    }
	  	  }
	  	  if(m)
	          {
	            //printf(" del lcmb:%s\n",lcmb[k].name);
                    delLcmb( 1, &k );
	          }
	        }
	      }
            }
	  }
	}
      }
      else
      {
	ll=l2;
	//printf(" %d check lcmb %s\n",j, lcmb[ll].name);
        for(c=0; c<lcmb[ll].nl; c++)
        {
          l2=lcmb[ll].l[c];
          //printf(" %d check line %s\n",j, line[l2].name);
          k=getIndex(&set[setSplit].line,set[setSplit].anz_l,l2);
          if(k>-1)
          {
            //printf(" %s matches splitline\n",line[set[setSplit].line[k]].name);
            surf[s2].l[j]=dep_l[l2];
            l2=surf[s2].l[j];
          }
          else
  	  {
            // change points
            //printf(" change points in line %s ltyp %c\n",line[l2].name, line[l2].typ);
            p1=p2=-1;
            if( dep_p[line[l2].p1]!=-1) { p1=line[l2].p1; line[l2].p1=dep_p[line[l2].p1]; }
            if( dep_p[line[l2].p2]!=-1) { p2=line[l2].p2; line[l2].p2=dep_p[line[l2].p2]; }
  	    if (line[l2].typ=='a') { if( dep_p[line[l2].trk]!=-1) line[l2].trk=dep_p[line[l2].trk]; }
            else if (line[l2].typ=='s')
            {
              se=line[l2].trk;
              if(p1>-1)
              {
                //printf(" line.p1 %s was updated, update trk %s\n",point[p1].name, set[se].name);
                for(k=0; k<set[se].anz_p; k++)
                {
                  if(set[se].pnt[k]==p1)
                  { set[se].pnt[k]=line[l2].p1; printf(" trkp:%d %s\n",k,point[line[l2].p1].name); break; }
                }
              }
              if(p2>-1)
              {
                //printf(" line.p2 %s was updated, update trk %s\n",point[p2].name, set[se].name);
                for(k=0; k<set[se].anz_p; k++)
                {
                  if(set[se].pnt[k]==p2)
                  { set[se].pnt[k]=line[l2].p2; printf(" trkp:%d %s\n",k,point[line[l2].p2].name);  break; }
                }
              }
            }
          }
        }
      }
    }
  }

  // store all sh and Nurs of buf2 in setbuf and copy their points
  completeSet(set[buf2].name, "do");
  for(i=0; i<set[buf2].anz_sh; i++) seta(setbuf,"sh",set[buf2].shp[i]);
  for(i=0; i<set[buf2].anz_nurs; i++) seta(setbuf,"S",set[buf2].nurs[i]);
  completeSet(set[setbuf].name, "do");
  for(i=0; i<set[setbuf].anz_sh; i++) setr(buf2,"sh",set[setbuf].shp[i]);
  for(i=0; i<set[setbuf].anz_nurs; i++) setr(buf2,"S",set[setbuf].nurs[i]);
  for(i=0; i<set[setbuf].anz_p; i++) setr(buf2,"p",set[setbuf].pnt[i]);
  
  /* copy points */
  if(set[setbuf].anz_p>0)
  {
    if ( (dep_p = (int *)realloc((int *)dep_p, (anzGeo->p+1) * sizeof(int))) == NULL )
    { printf(" ERROR: realloc failed\n"); return; }
    for (i=0; i<anzGeo->p; i++) dep_p[i]=-1;
    for (i=0; i<set[setbuf].anz_p; i++)
    {
      p= getNewName( name, "p" );
      if ( p == -1 )
      { printf(" ERROR: could not create new point\n"); return; }
      p=set[setbuf].pnt[i];
      p=pnt( name, point[p].px,point[p].py,point[p].pz, 0 );
      if ( p <0 )
      { printf(" ERROR: could not create new point\n"); return; }
      dep_p[set[setbuf].pnt[i]] = p; /* remember the index of the new point */
      //seta( settrgt, "p", p );
    }
  }

  /* copy nurs */
  if(set[setbuf].anz_nurs>0)
  {
    if ( (dep_S = (int *)realloc((int *)dep_S, (anzGeo->nurs+1) * sizeof(int))) == NULL )
    {  printf("\n\n ERROR: realloc failed \n\n"); return; }

    b=set[setbuf].anz_nurs;
    for (s=0; s<b; s++)
    {
      S= getNewName( name, "S" );
      if ( S == -1 )
        { printf("copy: could not create new nurs\n"); return; }

      S=set[setbuf].nurs[s];

      if ((nurbs = (Nurbs *)realloc( (Nurbs *)nurbs, (anzGeo->nurs+1)*sizeof(Nurbs)) ) == NULL )
      { printf("\n\nERROR: realloc failure in Nurs, nurbs:%s not installed\n\n", name); return; }

      nr=anzGeo->nurs;
      hashNurs( sumAsci, name, nr );
      anzGeo->nurs++;
      if((nurbs[nr].name= (char *)malloc((strlen(name)+1)*sizeof(char))) == NULL )
      { printf("ERROR: malloc failed\n\n" ); return; }
      strcpy(nurbs[nr].name, name);

      if(printFlag) printf("copy %s to %s\n",nurbs[S].name,nurbs[nr].name);

      nurbs[nr].u_exp = nurbs[S].u_exp;
      nurbs[nr].v_exp = nurbs[S].v_exp;
      nurbs[nr].u_npnt= nurbs[S].u_npnt;
      nurbs[nr].v_npnt= nurbs[S].v_npnt;
      nurbs[nr].u_nknt= nurbs[S].u_nknt;
      nurbs[nr].v_nknt= nurbs[S].v_nknt;
      nurbs[nr].u_stride= nurbs[S].u_stride;
      nurbs[nr].v_stride= nurbs[S].v_stride;

      if ( (nurbs[nr].uknt = (GLfloat *)malloc( (nurbs[S].u_nknt+1) * sizeof(GLfloat))) == NULL )
        printf("\n\n ERROR: realloc failed uknt\n\n");
      if ( (nurbs[nr].vknt = (GLfloat *)malloc( (nurbs[S].v_nknt+1) * sizeof(GLfloat))) == NULL )
        printf("\n\n ERROR: realloc failed vknt\n\n");
      for(i=0; i<nurbs[nr].u_nknt; i++) { nurbs[nr].uknt[i]=nurbs[S].uknt[i]; }
      for(i=0; i<nurbs[nr].v_nknt; i++) { nurbs[nr].vknt[i]=nurbs[S].vknt[i]; }

      if ( (nurbs[nr].ctlpnt =
        (int **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(int *))) == NULL )
        printf("\n\n ERROR: malloc failed ctlpnt\n\n");
      for (i=0; i<nurbs[nr].u_npnt; i++)
      {
        if ( (nurbs[nr].ctlpnt[i] =
          (int *)malloc(  (nurbs[nr].v_npnt+1) * sizeof( int ))) == NULL )
          printf("\n\n ERROR: malloc failed ctlpnt[i]\n\n");
        for (j=0; j<nurbs[nr].v_npnt; j++)
          nurbs[nr].ctlpnt[i][j] = dep_p[nurbs[S].ctlpnt[i][j]];
      }

      if ( (nurbs[nr].weight =
        (float **)malloc(  (nurbs[nr].u_npnt+1) * sizeof(float *))) == NULL )
        printf("\n\n ERROR: malloc failed weight\n\n");
      for (i=0; i<nurbs[nr].u_npnt; i++)
      {
        if ( (nurbs[nr].weight[i] =
          (float *)malloc(  (nurbs[nr].v_npnt+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: malloc failed weight[i]\n\n");
        for (j=0; j<nurbs[nr].v_npnt; j++)
          nurbs[nr].weight[i][j] =nurbs[S].weight[i][j];
      }

      nurbs[nr].ctlarray=(GLfloat *)NULL;
      nurbs[nr].endFlag=1;       
      nurbs[nr].type=GL_MAP2_VERTEX_4;       
  
      /* additional variables for the trimming */
      nurbs[nr].trimFlag=0;
      nurbs[nr].patches=0;
      nurbs[nr].nc=NULL;
      nurbs[nr].uv=NULL;
      nurbs[nr].xyz=NULL;
      nurbs[nr].np=NULL;
      nurbs[nr].umax=NULL;
      nurbs[nr].vmax=NULL;
      nurbs[nr].vstep=NULL;
      nurbs[nr].ustep=NULL;
      nurbs[nr].Nurb = (GLUnurbsObj *)gluNewNurbsRenderer();
      nurbs[nr].nurbsType=1;
      nurbs[nr].uvflipped=NULL;
      nurbs[nr].sum_ambiguousPnts=NULL;
      for (i=0; i<anz->sets; i++)
      {
        if ( set[i].flag=='o') seta( i, "S", nr );
      }
      repNurs(nr);

      dep_S[S] = nr; 
      //seta( settrgt, "S", nr );

      /* add to all sets were the orig is a member */
      for(se=1; se<anz->sets; se++) if((se!=settrgt)&&(se!=setbuf)&&(set[se].name!=(char *)NULL)&&(set[se].name[0]!='-')) if(!set[se].type) if( getIndex(&set[se].nurs, set[se].anz_nurs, S) >-1) { seta( se, "S", nr );  }
    }
  }

  /* copy the shapes */
  if(set[setbuf].anz_sh>0)
  {
    if ( (dep_sh = (int *)realloc((int *)dep_sh, (anzGeo->sh+1) * sizeof(int))) == NULL )
    {  printf("\n\n ERROR: realloc failed\n\n"); return; }
    for (i=0; i<anzGeo->sh; i++) dep_sh[i]=-1;
    
    j=set[setbuf].anz_sh;
    for (i=0; i<j; i++)
    {
      if (set[setbuf].shp[i]==-1)
      {
        errMsg(" Shapenr:%d is undefined\n", set[setbuf].shp[i] );
        return;
      }
      l=-1;

      if((shape[set[setbuf].shp[i]].type==0)||(shape[set[setbuf].shp[i]].type==1))
      {
        l= getNewName( name, "sh" );
        if ( l == -1 )
        { printf("copy: could not create new shape\n"); return; }
        p1=shape[set[setbuf].shp[i]].p[0];
        p2=shape[set[setbuf].shp[i]].p[1];
        p3=shape[set[setbuf].shp[i]].p[2];
        l=shape_i( name, shape[set[setbuf].shp[i]].type, dep_p[p1], dep_p[p2], dep_p[p3], 0, 0, 0, 0 );
      } 
      else if(shape[set[setbuf].shp[i]].type==2)
      {
        l= getNewName( name, "sh" );
        if ( l == -1 )
        { printf("copy: could not create new shape\n"); return; }
        p1=shape[set[setbuf].shp[i]].p[0];
        p2=shape[set[setbuf].shp[i]].p[1];
        p3=shape[set[setbuf].shp[i]].p[2];
        p4=shape[set[setbuf].shp[i]].p[3];
        l=shape_i( name, shape[set[setbuf].shp[i]].type, dep_p[p1], dep_p[p2], dep_p[p3], dep_p[p4], 0, 0, 0 );
      } 
      else if(shape[set[setbuf].shp[i]].type==3)
      {
        l= getNewName( name, "sh" );
        if ( l == -1 )
        { printf("copy: could not create new shape\n"); return; }
        p1=shape[set[setbuf].shp[i]].p[0];
        p2=shape[set[setbuf].shp[i]].p[1];
        p3=shape[set[setbuf].shp[i]].p[2];
        p4=shape[set[setbuf].shp[i]].p[3];
        p5=shape[set[setbuf].shp[i]].p[4];
        p6=shape[set[setbuf].shp[i]].p[5];
        p7=shape[set[setbuf].shp[i]].p[6];
        l=shape_i( name, shape[set[setbuf].shp[i]].type, dep_p[p1], dep_p[p2], dep_p[p3], dep_p[p4], dep_p[p5], dep_p[p6],  dep_p[p7] );
      } 
      else if(shape[set[setbuf].shp[i]].type==4)
      {
        p1=shape[set[setbuf].shp[i]].p[0];
        if(p1>-1)
          l=shape_i( nurbs[dep_S[p1]].name, shape[set[setbuf].shp[i]].type, dep_S[p1], 0,0,0, 0,0,0);
      } 
      else if(shape[set[setbuf].shp[i]].type==5)
      {
        l= getNewName( name, "sh" );
        if ( l == -1 )
        { printf("copy: could not create new shape\n"); return; }
        p1=shape[set[setbuf].shp[i]].p[0];
        p2=shape[set[setbuf].shp[i]].p[1];
        p3=shape[set[setbuf].shp[i]].p[2];
        p4=shape[set[setbuf].shp[i]].p[3];
        l=shape_i( name, shape[set[setbuf].shp[i]].type, dep_p[p1], dep_p[p2], dep_p[p3], dep_p[p4], 0, 0, 0 );
      } 
      if ( l <0 )
      { printf("copy: could not create new shape\n"); return; }
      dep_sh[set[setbuf].shp[i]] = l; /* remember the index of the new shape */
      //seta( settrgt, "sh", l );

      /* add to all sets were the orig is a member */
      for(se=1; se<anz->sets; se++) if((se!=settrgt)&&(se!=setbuf)&&(set[se].name!=(char *)NULL)&&(set[se].name[0]!='-')) if(!set[se].type) if( getIndex(&set[se].shp, set[se].anz_sh, set[setbuf].shp[i]) >-1) { seta( se, "sh", l );  }
    }
  }

  /* finally include all surfaces in buf1 and buf2 which are connected with them */

  //printf("- include all surfaces in set1 and set2 which are connected with them\n");
  
  if( (surfused = (int *)calloc((anzGeo->s+1),sizeof(int) ))==NULL )
    printf("ERROR: calloc failure\n");

  delSet( "-setbuf" );
  setbuf=pre_seta("-setbuf","i",0);
  for(i=0; i<set[set1].anz_s; i++) {  surfused[set[set1].surf[i]]=1; }
  for(i=0; i<set[set2].anz_s; i++) {  surfused[set[set2].surf[i]]=1; }
  for(i=0; i<anzGeo->s; i++) { if((!surfused[i])&&(surf[i].name!=NULL)) seta(setbuf, "s", i); }

  do{
    s=set[set1].anz_s;
    for(i=0; i<set[set1].anz_s; i++)
    {
      s1=set[set1].surf[i];
      //printf("%d check surf:%s\n",i,surf[s1].name);
      for(j=0; j<surf[s1].nl; j++)
      {
        l1=surf[s1].l[j];
	/*
        if(surf[s1].typ[j]=='l')
        {
    	  printf(" %d check line %s\n",j, line[l1].name);
        }
        else
        {
    	  printf(" %d check lcmb %s\n",j, lcmb[l1].name);
        }
	*/
        for(k=0; k<set[setbuf].anz_s; k++)
        {
          s2=set[setbuf].surf[k];
    	  if(surfused[s2]==1) continue;
          //printf("%d compare surf:%s\n",k,surf[s2].name);
          for(l=0; l<surf[s2].nl; l++)
          {
            l2=surf[s2].l[l];
            if((l1==l2)&&(surf[s1].typ[j]==surf[s2].typ[l]))
            {
              //printf(" %d matches %d\n",l1,l2);
    	      seta(set1, "s", s2);
    	      surfused[s2]=1;
    	    }
    	  }
        }
      }
    }
  }while(s < set[set1].anz_s);

  do{
    s=set[set2].anz_s;
    for(i=0; i<set[set2].anz_s; i++)
    {
      s1=set[set2].surf[i];
      //printf("%d check surf:%s\n",i,surf[s1].name);
      for(j=0; j<surf[s1].nl; j++)
      {
        l1=surf[s1].l[j];
	/*
        if(surf[s1].typ[j]=='l')
        {
  	  printf(" %d check line %s\n",j, line[l1].name);
        }
        else
        {
  	  printf(" %d check lcmb %s\n",j, lcmb[l1].name);
        }
	*/
        for(k=0; k<set[setbuf].anz_s; k++)
        {
          s2=set[setbuf].surf[k];
  	  if(surfused[s2]==1) continue;
          //printf("%d compare surf:%s %d\n",k,surf[s2].name, surf[s2].nl);
          for(l=0; l<surf[s2].nl; l++)
          {
            l2=surf[s2].l[l];
            if((l1==l2)&&(surf[s1].typ[j]==surf[s2].typ[l]))
            {
              //printf(" %d matches %d\n",l1,l2);
  	      seta(set2, "s", s2);
  	      surfused[s2]=1;
  	    }
  	  }
        }
      }
    }
  }while(s < set[set2].anz_s);
  delSet( "-setbuf" );
  delSet( "-buf1" );
  delSet( "-buf2" );
    
  /* replace the referenced shapes */
  
  //printf("- replace the referenced shapes\n");
  
  for(i=0; i<set[set2].anz_s; i++)
  {
    s2=set[set2].surf[i];
    if((surf[s2].sh>-1)&&(dep_sh[surf[s2].sh]>-1))
    {
      surf[s2].sh=dep_sh[surf[s2].sh];
      //printf("%d mod surf:%s sh:%d\n",i,surf[s2].name,surf[s2].sh);
    }
  }

  completeSet(set[set1].name, "do");
  completeSet(set[set2].name, "do");

  // the set +cut1 might be distributed between +set1 and +set2, same for +cut2
  // go over all lines from +cut1 and check if it is part of +set2
  // if yes, remove from +cut1 and add to +cut2
  // go over all lines from +cut2 and check if it is part of +set1
  // if yes, remove from +cut2 and add to +cut1
  for(i=0; i<set[setSplit].anz_l; i++)
  {
    l=set[setSplit].line[i];
    l1=getIndex(&set[set2].line,set[set2].anz_l,l);
    if(l1>-1)
    {
      setr(setSplit,"l", l); i--;
      seta(settrgt,"l", l);
    }
  }
  for(i=0; i<set[settrgt].anz_l; i++)
  {
    l=set[settrgt].line[i];
    l1=getIndex(&set[set1].line,set[set1].anz_l,l);
    if(l1>-1)
    {
      setr(settrgt,"l", l); i--;
      seta(setSplit,"l", l);
    }
  }
  
  free(surfused);
    
  free(dep_p);
  free(dep_l);
  free(dep_sh);
  free(dep_S);
}



#define TESTX2 0
/*
output:
  return: nrOfSurfs created
  ptr_innerSurf: array with new surfaces based on nls lines  
 */
int surfsFromCut(int nls, int *ls, int shp, int **ptr_innerSurf)
{
  int e,i,ii,j=0,k, n=0, nn,p, snew=0, nsurfs=0;
  int     flag1, flag2, err=0, lcount=0, lastlcount, closedLoopFlag;
  int     p1,p2, pnt=0, sl, cl, l0;
  int     line0_p1, line0_p2;
  int     nc=0;             /* number of curves */
  int     *cnr=NULL;        /* number of lines in each curve */
  int     *sortLines=NULL;
  char    *sortOri=NULL, *tmpOri=NULL, *sortTyp=NULL;
  char    name[MAX_LINE_LENGTH];
  int     *innerSurf=NULL, *cused=NULL, *clenindx=NULL;
  char    *os=NULL, *typs=NULL;
  double  *clength=NULL;
  Rsort   *rsort=NULL;

  typedef struct{
    int *l;
    char *o, *t;
  }Curve;
  Curve *curve=NULL;
  
  int patch, Stmp, sh_buf, nurbsnr, np, npc;
  double pnt5v[5], *pnt_u=NULL, *pnt_v=NULL;
  int    *pnt_flag=NULL;
  int    *tri=NULL;
  
  innerSurf=*ptr_innerSurf;
#if TESTX2    
  for (i=0; i<nls; i++)
  {
    printf(" l:%d %s\n",i,line[ls[i]].name);
  }
#endif
  if(nls<1) return(-1);


  /**** create closed line loops ***/
  
  if ((os = (char *)realloc(os, (nls+1)*sizeof(char)) ) == NULL )
  { printf("ERROR: malloc failure\n"); return(-1); }
  if ((typs = (char *)realloc(typs, (nls+1)*sizeof(char)) ) == NULL )
  { printf("ERROR: malloc failure\n"); return(-1); }

  if((sortOri = (char *)realloc((char *)sortOri, (nls)*sizeof(char)) ) == NULL )
  { printf(" ERROR: realloc failure in orientSurf()\n\n"); return(-1); }
  if((sortTyp = (char *)realloc((char *)sortTyp, (nls)*sizeof(char)) ) == NULL )
  { printf(" ERROR: realloc failure in orientSurf()\n\n"); return(-1); }
  if((sortLines=(int *)realloc((int *)sortLines, (nls)*sizeof(int) ) )==NULL)
  { printf(" ERROR: realloc failure in orientSurf()\n\n"); return(-1); }
  if((tmpOri = (char *)realloc((char *)tmpOri, (nls)*sizeof(char)) ) == NULL )
  { printf(" ERROR: realloc failure in orientSurf()\n\n"); return(-1); }

  /* loesche alle linien orientierungen */
  for (i=0; i<nls; i++)
  {
    tmpOri[i]='+';
    os[i]=0;
    typs[i]='l';
  }

  do
  {
    lastlcount=lcount;
#if TESTX2    
    printf("\n curve starts with n:%d lcount:%d \n",n,lcount);
#endif

    /* suche den punkt einer linie die mit der ersten linie verbunden ist */
    /* und orientiere die erste linie */
    l0=ls[n];
    line0_p1=line[l0].p1;
    line0_p2=line[l0].p2;

    /* gehe durch alle Linien und suche die zweite Linie */
    flag1=flag2=0;
    for (i=n+1; i<nls; i++)
    {
      p1=line[ls[i]].p1;
      p2=line[ls[i]].p2;
#if TESTX2    
      printf ("l:%s  p1:%s p2:%s  \n", line[ls[i]].name, point[p1].name, point[p2].name );
#endif
  
      if(tmpOri[n]=='+')
      {
        if      ( line0_p2 == p2 ) flag2=2;
        else if ( line0_p2 == p1 ) flag2=1;
        else if ( line0_p1 == p2 ) flag1=2;
        else if ( line0_p1 == p1 ) flag1=1;
      }
      else
      {
        if      ( line0_p1 == p2 ) flag1=2;
        else if ( line0_p1 == p1 ) flag1=1;
        else if ( line0_p2 == p2 ) flag2=2;
        else if ( line0_p2 == p1 ) flag2=1;
      }

      if (flag1==1)
      {
          os[n]='-';
          os[i]='+';
          pnt=p2;
          break;
      }
      else if (flag1==2)
      {
          os[n]='-';
          os[i]='-';
          pnt=p1;
          break;
      }
      else if (flag2==1)
      {
          os[n]='+';
          os[i]='+';
          pnt=p2;
          break;
      }
      else if (flag2==2)
      {
          os[n]='+';
          os[i]='-';
          pnt=p1;
          break;
      }
      else pnt=0;
    }

    if (i==nls)
    {
      if(typs[n]=='l') errMsg("WARNING: line[%d]:%s not connected\n",n,line[ls[n]].name);
      //typs[n]='d';
      if(typs[n]=='l')  typs[n]='L'; else typs[n]='C';
      n++;
      if(n==nls)
      {
        errMsg("WARNING: surf not closed, no orientation possible\n");
        for (i=0; i<nls; i++)
        {
          os[i]='+';
        }
	err=-1;
        goto orientSurfError;
      }
      else continue;
    }
    else
    {
      sortLines[lcount]=ls[n];      /* merke dir die reihenfolge in der die linien kommen */
      sortOri[lcount]=  os[n];      /* merke dir die dazugehoerigen orientationen */
      sortTyp[lcount]=  typs[n];
      lcount++;
      sortLines[lcount]=ls[i];      /* merke dir die reihenfolge in der die linien kommen */
      sortOri[lcount]=  os[i];      /* merke dir die dazugehoerigen orientationen */
      sortTyp[lcount]=  typs[i];
      lcount++;
    }

#if TESTX2    
    printf ("Curve:%d Start-line:%s ori:%c 2nd.line:%s ori:%c \n",nc,line[ls[n]].name, os[n], line[ls[i]].name, os[i] );
#endif
    
    closedLoopFlag=0;
    for (i=0; i<nls; i++) /* suche die anschliessende durch vergleich (with inner second loop!)*/
    {
      for (j=n+1; j<nls; j++) /* suche die anschliessende durch vergleich */
      {
        if (!os[j])              /* wenn ori noch == 0 */
        {
#if TESTX2    
          printf ("check pnt:%s pnt:%d j:%d\n", point[pnt].name, pnt, j);
#endif
          if (typs[j]=='c')
          {
            sl=ls[j];
            cl=lcmb[sl].nl-1;
            if(lcmb[sl].o[0]=='+')     p1=line[lcmb[sl].l[0]].p1;
            else                       p1=line[lcmb[sl].l[0]].p2;
            if(lcmb[sl].o[cl]=='+')   p2=line[lcmb[sl].l[cl]].p2;
            else                      p2=line[lcmb[sl].l[cl]].p1;
#if TESTX2    
          printf ("compare i:%d j:%d lcmb:%s pnt:%d p1:%d p2:%d\n", i,j, lcmb[ls[j]].name, pnt, p1, p2 );
#endif
          }
          else
          {
            p1=line[ls[j]].p1;
            p2=line[ls[j]].p2;
#if TESTX2    
          printf ("compare i:%d j:%d line:%s ori:%c pnt:%d p1:%d p2:%d\n", i,j, line[ls[j]].name, os[j], pnt, p1, p2 );
#endif
          }

          if ( p1 == pnt )
          {
	    pnt=p2; os[j]='+';
#if TESTX2    
          printf ("found i:%d j:%d p1:%d p2:%d  next pnt:%d\n", i,j, p1, p2, pnt );
#endif
            sortLines[lcount]=ls[j]; sortOri[lcount]=os[j];
            sortTyp[lcount]=typs[j]; lcount++;
          }
          else if ( p2 ==  pnt )
          {
	    pnt=p1; os[j]='-';
#if TESTX2    
          printf ("found i:%d j:%d  p1:%d p2:%d  next pnt:%d\n", i,j, p1, p2, pnt );
#endif
            sortLines[lcount]=ls[j]; sortOri[lcount]=os[j];
            sortTyp[lcount]=typs[j]; lcount++;
          }
        }
      }

#if TESTX2    
      printf ("check connected lines\n");
#endif

      /* check if the curve is closed  */
      if(typs[n]=='l')
      {
        if((pnt==line[ls[n]].p1)||(pnt==line[ls[n]].p2))
        {
          /* number of lines in each curve */
          if((cnr=(int *)realloc((int *)cnr, (nc+1)*sizeof(int)) )==NULL)
          { printf(" ERROR: realloc failure\n"); exit(-1); }
          cnr[nc]=lcount-lastlcount;
          nc++;
	  closedLoopFlag=1;
#if TESTX2    
          printf ("line, closed line loop found, n:%d\n",n);
#endif
        }
      }
      if(closedLoopFlag==1) break;
    }
    if(closedLoopFlag==0) lcount=lastlcount;

    
    /* check for unoriented lines */
    err=0;
    for (n=0; n<nls; n++)
    {
      //if( (!os[n])&&(typs[n]!='d'))
       if( (!os[n]) && ((typs[n]!='L')&&(typs[n]!='C')) )
       {
        err--;
        break;
      }
    }
  }while(err<0);

#if TESTX2    
  printf("lcount:%d nls:%d\n",lcount,nls);
#endif

  if(cnr==NULL) return(0);
  
  for(j=0; j<cnr[0]; j++)
  {
#if TESTX2    
    printf("c:%d l:%d %s %c %c\n",i, j,line[sortLines[j]].name, sortOri[j], sortTyp[j]);
#endif
    ls[j]=sortLines[j];
    os[j]=sortOri[j];
    typs[j]=sortTyp[j];
  }
  lcount=0;

  // nc curves are collected, each with cnr[i] of lines
  // determine the length of each curve, store in clength[i]
  // sort clength and store the index i. longest first
  // start loop:
  //   take the longest of which cused[i] is still 0 (unused)
  //   determine which of the remaining are inside of the longest using mesh2d (hollow surf)
  //   create a surface based on them and mark the curves as used: cused[i]=1.
  // loop

  // determine the length
  if ((clength = (double *)realloc(clength, (nc+1)*sizeof(double)) ) == NULL )
  { printf("ERROR: realloc failure\n");  }
  if ((cused = (int *)calloc( (nc+1), sizeof(int)) ) == NULL )
  { printf("ERROR: realloc failure\n");  }
  if ((clenindx = (int *)realloc(clenindx, (nc+1)*sizeof(int)) ) == NULL )
  { printf("ERROR: realloc failure\n");  }
  if ((curve = (Curve *)malloc((nc+1)*sizeof(Curve)) ) == NULL )
  { printf("ERROR: malloc failure\n");  }
  lcount=0;
  for(i=0; i<nc; i++)
  {
    printf("cnr[%d]:%d\n",i,cnr[i]);
    if ((curve[i].l = (int *)malloc((cnr[i])*sizeof(int)) ) == NULL )
    { printf("ERROR: malloc failure\n");  }
    if ((curve[i].o = (char *)malloc((cnr[i])*sizeof(char)) ) == NULL )
    { printf("ERROR: malloc failure\n");  }
    if ((curve[i].t = (char *)malloc((cnr[i])*sizeof(char)) ) == NULL )
    { printf("ERROR: malloc failure\n");  }
    clength[i]=0.;
    n=0;
    for(j=lcount; j<lcount+cnr[i]; j++)
    {
      //printf("c:%d l:%d %s %c %c\n",i, j,line[sortLines[j]].name, sortOri[j], sortTyp[j]);
      clength[i]+=calcLineLength(sortLines[j]);
      // store the curves in new arrays
      curve[i].l[n]=sortLines[j];
      curve[i].o[n]=sortOri[j];
      curve[i].t[n]=sortTyp[j];
      n++;
    }
    lcount+=cnr[i];
    printf("c:%d LENGTH:%e\n", i, clength[i]);
  }
  // sort clength and remember the index
  if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (nc+1) * sizeof(Rsort))) == NULL )
    printf("ERROR: realloc failed: Rsort\n\n" ); 
  for(i=0; i<nc; i++)
  {
    rsort[i].r=clength[i];
    rsort[i].i=i;
  }
  qsort( rsort, nc, sizeof(Rsort), (void *)compareRsort );
  n=0; for(i=nc-1; i>=0; i--)
  {
    clenindx[n++]=rsort[i].i;
#if TESTX2    
    printf("sorted c:%d LENGTH:%e\n", rsort[i].i, clength[rsort[i].i]);
#endif
  }
  free(clength);
  free(rsort);

  // innerSurf collects the new surfs
  if((innerSurf= (int *)realloc((int *)innerSurf, (nc+1)*sizeof(int))) == NULL )
  { printf("ERROR: realloc failure\n"); return(-1); }

  for(n=0; n<nc; n++)
  {
    i=clenindx[n];
    if(cused[i]==1) continue;
    cused[i]=1;

#if TESTX2
    printf("cnr[%d]:%d\n",i,cnr[i]);
    for(j=0; j<cnr[i]; j++)
    {
      printf("c:%d l:%d %s %c %c\n",i, j,line[curve[i].l[j]].name, curve[i].o[j], curve[i].t[j]);
    }
#endif
    getNewName( name, "s" );
    snew= surface_i( name, '+', shp, cnr[i], curve[i].o, curve[i].l, curve[i].t );
    if(snew<0) return(0);
    innerSurf[nsurfs]=snew;
    nsurfs++;
#if TESTX2    
    printf(" 1 snew:%d  surfnr:%d %s created\n", snew, nsurfs, name);
#endif

    // shp has to be meshable with mesh2d 
    if(shp>-1)
    {
      // mesh the surf, use then the uv coordinates 

      /* if shape, generate prelim nurbs */
      sh_buf=-1;
      if(shape[surf[snew].sh].type>=0) Stmp= surfToNurs(snew);
      if(Stmp>-1)
      {
        sh_buf=surf[snew].sh;
        surf[snew].sh=shape_i( nurbs[Stmp].name, 4, Stmp, 0, 0, 0, 0,0,0);
        if(printFlag) printf (" interior changed to Nurbs: %s\n", nurbs[Stmp].name );
      }
      
      if(shape[surf[snew].sh].type==4)
      {
        nurbsnr=shape[surf[snew].sh].p[0];
  
        /* calculate the xyz and uv values and orientation of the trimming loops of the surface (patch)  */
        ii=calcTrimLoops(nurbsnr, snew);
        if(ii!=0)
        {
          if(ii>0)
  	  {
            printf(" ERROR: surf:%s could not be trimmed. All points are located on ambiguous edges. Try to fix the geometry manually.\n",surf[snew].name);
            surf[snew].fail=2;
  	  }
          goto nurbsCouldNotBeTrimmed;
        }
      }
      else goto nurbsCouldNotBeTrimmed;

      patch=surf[snew].patch;
      npc=nurbs[nurbsnr].np[patch][0]-1;
      if( (pnt_u = (double *)malloc( (npc+1)*sizeof(double) )) == NULL )
      { printf(" ERROR: malloc failure\n\n");
        return(-1); }
      if( (pnt_v = (double *)malloc( (npc+1)*sizeof(double) )) == NULL )
      { printf(" ERROR: malloc failure\n\n");
        return(-1); }
      np=nn=0;
      for(j=0; j<npc; j++)
      {
        pnt_u[np]=nurbs[nurbsnr].uv[patch][0][nn++];
        pnt_v[np]=nurbs[nurbsnr].uv[patch][0][nn++];
	np++;
      }
      /*
      for(nn=0; nn<np; nn++)
      {
	printf(" pnt ! %f %f\n",pnt_u[nn],pnt_v[nn]);
      }
      */
      k=1;
      e=mesh2d(&np, k, &npc, &pnt_u, &pnt_v, &pnt_flag, &tri, 0.4, 0.4, 1 );
      /*
      printf("seto mesh\n");
      for(nn=0; nn<np; nn++)
      {
	printf(" pnt ! %f %f\n",pnt_u[nn],pnt_v[nn]);
      }
      */
      
      // if a line-point of another curve lies in the inner triangle of the surface, then
      // add the curve to the line buffers of this surface and update it
    
      // go over all curves and look if one is inside
      for(nn=0; nn<nc; nn++)
      {
        ii=clenindx[nn];
#if TESTX2    
        printf("cnr[%d] used:%d\n",ii,cused[ii]);
#endif
	if(cused[ii]==1) continue;

        //  check endpoints of ii if inside i
	flag1=0;
	for(j=0; j<cnr[ii]; j++)
        {
          // get uv coordinates of line-point
          if(curve[ii].o[j]=='+') p=line[curve[ii].l[j]].p1;
	  else p=line[curve[ii].l[j]].p2;
          pnt5v[0]=point[p].px;
          pnt5v[1]=point[p].py;
          pnt5v[2]=point[p].pz;
          proj1PntToNurbs( nurbsnr, pnt5v);
	  flag1=check_lineLoop(&pnt5v[3], np, pnt_u, pnt_v, e, tri);
#if TESTX2    
          printf("inside:%d uv %f %f\n", flag1, pnt5v[3], pnt5v[4]);
#endif
	  if(flag1) break;
        }
	if(flag1)
	{
          // curve is inside, add the members to curve i
          cused[ii]=1;
	  if ((curve[i].l = (int *)realloc(curve[i].l,(cnr[i]+cnr[ii])*sizeof(int)) ) == NULL )
          { printf("ERROR: malloc failure\n");  }
          if ((curve[i].o = (char *)realloc(curve[i].o,(cnr[i]+cnr[ii])*sizeof(char)) ) == NULL )
          { printf("ERROR: malloc failure\n");  }
          if ((curve[i].t = (char *)realloc(curve[i].t,(cnr[i]+cnr[ii])*sizeof(char)) ) == NULL )
          { printf("ERROR: malloc failure\n");  }
          for(j=0; j<cnr[ii]; j++)
          {
	    curve[i].l[j+cnr[i]]=curve[ii].l[j];
	    curve[i].o[j+cnr[i]]=curve[ii].o[j];
	    curve[i].t[j+cnr[i]]=curve[ii].t[j];
	    //printf("c:%d l:%d %s %c %c\n",i, j,line[curve[i].l[j]].name, curve[i].o[j], curve[i].t[j]);
          }
          cnr[i]+=cnr[ii];
	  
          // and update the surface snew
          //for(j=0; j<cnr[i]; j++) printf("c:%d l:%d %s %c %c\n",i, j,line[curve[i].l[j]].name, curve[i].o[j], curve[i].t[j]);
          snew= surface_i( name, '+', shp, cnr[i], curve[i].o, curve[i].l, curve[i].t );
          if(snew<0) return(0);
          //printf("2 snew:%d  surfnr:%d %s created\n",snew, nsurfs, name);
        }
      }
      
    nurbsCouldNotBeTrimmed:;
      /* restore the pointer to the shape */
      if(sh_buf>-1)
      {
        surf[snew].sh=sh_buf;
        for (i=0; i<nurbs[Stmp].u_npnt; i++) delPnt( nurbs[Stmp].v_npnt, nurbs[Stmp].ctlpnt[i] );
        delNurs( 1, &Stmp );
      }
    }

  }
  *ptr_innerSurf=innerSurf;
  free(cused);
  free(clenindx);
  for(i=0; i<nc; i++) { free(curve[i].l); free(curve[i].o); free(curve[i].t); }
  free(curve);
  free(pnt_u);
  free(pnt_v);
  free(pnt_flag);
  free(tri);
 orientSurfError:;
  free(sortLines);
  free(sortOri);
  free(tmpOri);
  free(sortTyp);
  free(cnr);
  return(nsurfs);
}



void pre_split( char *record)
{
  int  b,c,e,i,j=0,k,kk,kkbuf,n,nn,m,p,ll,l,d1,d2,ss,s, setNr, trgtNr, trkNr, mode, ibuf=0, orient=0;
  char setname[MAX_LINE_LENGTH], targetname[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH], buffer[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH], eparm[MAX_LINE_LENGTH];
  double pl[2][3],ps[3][3], pc[3], psplt[10][3], vproj[3], dist=0., dist1=0., dist2=0., distn=0., distbuf=MAX_FLOAT, lp0p1, lps;
  double pt1[3], pt2[3], pt3[3], vs[3], p01[3], eu[3], ev[3], eg[3], en[3], g;
  double pp1[3],pp2[3];
  double cg_edge[3][3];

  int p1_nr, p2_nr, ps_nr, l_nr[2], s_nr[2], sum_psplt, setSplit, setMesh, setSurf;
  int *lbuffer=NULL;
  int sum_l=0;
  int *delpnts=NULL;

  Rsort *rsort=NULL;
  Tri *tri=NULL;
  int sum_tri=0;
  float dx,dy,dz;

  int   ii, jj=0, ns, nm, nc=0, n1, n2, ee, ec, en1, en2, nn1, nn2, setdep, setind, nunspl[6][3];
  double  vn_tri[12][3], vn_av[3], **pntbuf;
  int  set_efail, set_eorig=0, set_etop=0, set_ebot=0, set_ntop=0, set_nbot=0, ecounter=0;
  int  msgFlag=0, linoffs;
  int *usedLoops=NULL;

  int  nodseq_te4[]={0,1,1,2,2,0, 0,3,1,3,2,3};
  int  nodseq_te10[]={0,4,1,1,5,2,2,6,0, 0,7,3,1,8,3,2,9,3};
  int  nodseq_pe6[]={0,1,1,2,2,0, 3,4,4,5,5,3, 0,3,1,4,2,5};
  int  nodseq_pe15[]={0,6,1,1,7,2,2,8,0, 3,12,4,4,13,5,5,14,3, 0,9,3,1,10,4,2,11,5};
  int  edge[20], nnew[20], ncpy[20], nel[20], nbuf[20], nref[20][2];
  double v12[3], v13[3], v15[3], v15n[3], vn[3];
  double bv15=0., bvn, bv15n, bgrenz, ltol, L1,L2,ql;
  int  ncollapsed[27], ltyp, lnr;

  int     n_closest_nodes, meshAgainFlag=0;
  double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL, near_node[N_CLOSEST_TRI];

  typedef struct {
      int sum, *n2, *nm, *nc;
  }N1nm;
  N1nm *n1ns;
  N1nm *n1nm;

  typedef struct {
      int p, l;
  }Splitbuf;
  Splitbuf *splitbuf=NULL;
  int sum_splitbuf=0;

  int *surfp=NULL;
  int sum_surfp=0;

  int *linep=NULL;
  int sum_linep=0;

  typedef struct {
    int s, nl, *l, *used;
  }Surfbuf;
  Surfbuf *surfbuf=NULL;
  int sum_surfbuf=0;

  typedef struct {
    int nl, *l;
  }Linebuf;
  Linebuf *linebuf=NULL;
  int sum_linebuf=0;

  int setp, sets, setB1, nsurfs;
  int l1,l2,s1,i1,i2,flipFlag;
  int *innerSurf=NULL;

  delSet("-splitdep");
  delSet("-splitind");
  
  param[0]=0;
  ncollapsed[0]=-1;
  sscanf( record,"%s %s %s", setname, targetname, param );

  operateAlias( setname, "se" );
  setNr=getSetNr( setname );
  if (setNr<0)
  {
    // assume a line
    l=getLineNr( setname );
    if (l<0)
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
    if( (setdep=pre_seta( "-splitdep", "l", setname )) <0 ) return;
  }
  else
  {
    if(set[setNr].anz_b>1) { printf(" ERROR: splitting of bodies is currently restricted to one body. Be aware that splitting must not result in more than two bodies. Use several splits instead\n"); return; }
    
    /* cycle through all entities of setNr and add them to the special set  */
    /* cyrcle through all bodys and add  */
    if( (setdep=pre_seta( "-splitdep", "i", 0 )) <0 ) return;
    for (i=0; i<set[setNr].anz_b; i++)
    {
      seta( setdep, "b", set[setNr].body[i] );
    }
    /* cyrcle through all surfs and add  */
    for (i=0; i<set[setNr].anz_s; i++)
    {
      seta( setdep, "s", set[setNr].surf[i] );
    }
    /* cyrcle through all lcmbs and add  */
    for (i=0; i<set[setNr].anz_c; i++)
    {
      seta( setdep, "c", set[setNr].lcmb[i] );
    }
    /* cyrcle through all lines and add  */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      seta( setdep, "l", set[setNr].line[i] );
    }
    for (i=0; i<set[setNr].anz_p; i++)
    {
      seta( setdep, "p", set[setNr].pnt[i] );
    }
    for (i=0; i<set[setNr].anz_e; i++)
    {
      seta( setdep, "e", set[setNr].elem[i] );
    }
  }
  /* second cycle through all entities and add lower ones  to the special set  */
  completeSet( "-splitdep", "do") ;

  
  /* first assume the target is a point */
  operateAlias( targetname, "p" );
  ps_nr=getPntNr( targetname );
  if (ps_nr>-1)
  {
    // point given, create two new lines
    splitLine(l, l_nr, ps_nr);

    /* loesche basislinie */
    delLine( 1, &l );
    return;
  }

  /* assume the target is a single shape */
  operateAlias( targetname, "sh" );
  trgtNr=getShapeNr( targetname );
  if (trgtNr>-1)
  {
    if( (setind=pre_seta( "-splitind", "sh", targetname )) <0 ) return;
    trgtNr=-1;
  }
  else
  {
    operateAlias( targetname, "se" );
    trgtNr=getSetNr( targetname );
    if (trgtNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", targetname );
      return;
    }

    /* cycle through all entities of targetNr and add them to the special set  */
    /* cyrcle through all bodies and add  */
    if( (setind=pre_seta( "-splitind", "i", 0 )) <0 ) return;
    for (i=0; i<set[trgtNr].anz_s; i++)
    {
      seta( setind, "s", set[trgtNr].surf[i] );
    }
    for (i=0; i<set[trgtNr].anz_sh; i++)
    {
      seta( setind, "sh", set[trgtNr].shp[i] );
    }
    for (i=0; i<set[trgtNr].anz_l; i++)
    {
      seta( setind, "l", set[trgtNr].line[i] );
    }
    for (i=0; i<set[trgtNr].anz_f; i++)
    {
      seta( setind, "f", set[trgtNr].face[i] );
    }
    for (i=0; i<set[trgtNr].anz_sh; i++)
    {
      seta( setind, "sh", set[trgtNr].shp[i] );
    }
  }
  /* second cycle through all entities and add lower ones to the special set  */
  completeSet( "-splitind", "do") ;

  if (set[setNr].anz_n==0) set[setdep].anz_n=set[setNr].anz_n;
  setNr=setdep;
  trgtNr=setind;

  /* fill all unfilled surfaces, do it in any case because points could have been moved! */
  /* repShape will change *scale and therefore must be executed first */
  repShape(trgtNr);
  for (i=0; i<set[setNr].anz_l; i++) repLine(set[setNr].line[i]);
  for (j=0; j<set[trgtNr].anz_s; j++)
  {
    if(surf[set[trgtNr].surf[j]].sh>-1)
    {
      if(shape[surf[set[trgtNr].surf[j]].sh].type==4)
      {
        repNurs(shape[surf[set[trgtNr].surf[j]].sh].p[0]);
        untrimNurs(shape[surf[set[trgtNr].surf[j]].sh].p[0]);
      }
    }
    repSurf(set[trgtNr].surf[j],1);
  }
  for (j=0; j<set[setNr].anz_s; j++)
  {
    if(surf[set[setNr].surf[j]].sh>-1)
    {
      if(shape[surf[set[setNr].surf[j]].sh].type==4)
      {
        repNurs(shape[surf[set[setNr].surf[j]].sh].p[0]);
        untrimNurs(shape[surf[set[setNr].surf[j]].sh].p[0]);
      }
    }
    
    // splitting requires a high resolution of the surfaces, increase the density by using tri3 with increased inner density
    dist=0.5;
    // change:
    e=surf[set[setNr].surf[j]].etyp;
    if(surf[set[setNr].surf[j]].eparm!=(char *)NULL) strcpy(eparm,surf[set[setNr].surf[j]].eparm); else eparm[0]=0;
    surf[set[setNr].surf[j]].etyp=7;
    if(surf[set[setNr].surf[j]].eparm!=(char *)NULL) sprintf(surf[set[setNr].surf[j]].eparm,"%.3f", atof(surf[set[setNr].surf[j]].eparm) * dist);
    else
    {
      sprintf(buffer,"%.3f", dist);
      if( (surf[set[setNr].surf[j]].eparm=(char *)realloc((char *)surf[set[setNr].surf[j]].eparm, (strlen(buffer)+1)*sizeof(char))) == NULL )
        printf("\n\n ERROR: realloc failed\n\n") ;
      sprintf(surf[set[setNr].surf[j]].eparm,"%.3f", dist);
    }
    //printf("surf[set[setNr].surf[j]].eparm:%s\n",surf[set[setNr].surf[j]].eparm);
    repSurf(set[setNr].surf[j],1);
    // redefine:
    surf[set[setNr].surf[j]].etyp=e;
    if(eparm[0]==0) { free(surf[set[setNr].surf[j]].eparm); surf[set[setNr].surf[j]].eparm=NULL; }
    else strcpy(surf[set[setNr].surf[j]].eparm, param);
  }

  /* generate splitting triangles */
  sum_tri=genSplitTrias(trgtNr, &tri, 0);
  
  /*
  printf("sum_tri:%d\n",sum_tri);
  */

  ltol=gtol/scale->w;

  /* 0) split all elems */
  if(set[setdep].anz_e)
  {
    /* free the additional midside-nodes for higher order elements */
    for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
    anz->n= anz->orign;
    anz->nmax=anz->orignmax;

    set_efail=pre_seta("+efail","i",0);
    if(param[0]=='e') set_eorig=pre_seta("+eorig","i",0);
    else
    {
      set_etop=pre_seta("+etop","i",0);
      set_ebot=pre_seta("+ebot","i",0);
      set_ntop=pre_seta("+ntop","i",0);
      set_nbot=pre_seta("+nbot","i",0);
    }

    /* free the additional midside-nodes for higher order elements */
    for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
    anz->n= anz->orign;
    anz->nmax=anz->orignmax;

    /* create a table for all nodes which points to already created split nodes */
    if ( (n1ns = (N1nm *)malloc( (anz->nmax+1) * sizeof(N1nm))) == NULL )
    { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
    for (i=0; i<=anz->nmax; i++) n1ns[i].sum=0;
    for (i=0; i<=anz->nmax; i++) n1ns[i].n2=n1ns[i].nm=n1ns[i].nc=NULL;

    /* create a table for all nodes which points to already created midside nodes */
    if ( (n1nm = (N1nm *)malloc( (anz->nmax+1) * sizeof(N1nm))) == NULL )
    { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
    for (i=0; i<=anz->nmax; i++) n1nm[i].sum=0;
    for (i=0; i<=anz->nmax; i++) n1nm[i].n2=n1nm[i].nm=n1nm[i].nc=NULL;

    /* stelle daten fuer near3d bereit */
    if((int)N_CLOSEST_TRI<sum_tri) n_closest_nodes=(int)N_CLOSEST_TRI; else n_closest_nodes=sum_tri;
    if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
    if ( (orig_x = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (orig_y = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (orig_z = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_x = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_y = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_z = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_nx = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_ny = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_nz = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
  
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_x[i]=tri[i].cg[0];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_x[i]=rsort[i].r;
      sort_nx[i]=rsort[i].i;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_y[i]=tri[i].cg[1];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_y[i]=rsort[i].r;
      sort_ny[i]=rsort[i].i;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_z[i]=tri[i].cg[2];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_z[i]=rsort[i].r;
      sort_nz[i]=rsort[i].i;
    }

    /* go over all elements and calculate the split-point */
    for (ll=0; ll<set[setdep].anz_e; ll++)
    {
      e=set[setdep].elem[ll];
      //debug_pre_split printf("check elem[%d]:%d type:%d\n", ll, e, e_enqire[e].type);
      if(ecounter>set[setdep].anz_e/10)
      {
        printf("%d from %d elems processed\n", ll+1, set[setdep].anz_e);
        ecounter=0;
      }
      else ecounter++;

      if((e_enqire[e].type==3)||(e_enqire[e].type==6))
      {
        c=j=l=m=k=0;
        /* split the 6 edges and remember the new node per edge-nodes */
        for(i=0; i<6; i++)
        {
          en1=nodseq_te4[j++];
          en2=nodseq_te4[j++];
          n1=e_enqire[e].nod[en1];
          n2=e_enqire[e].nod[en2];
          pl[0][0]=node[n1].nx;
          pl[0][1]=node[n1].ny;
          pl[0][2]=node[n1].nz;
          pl[1][0]=node[n2].nx;
          pl[1][1]=node[n2].ny;
          pl[1][2]=node[n2].nz;
    
          /* The embedded triangles of the surfaces are used as a target */
          /* determine the position where the vector vproj between two line-dots meets the triangles */
          /* and determine if the position is inside the triangle */
          vproj[0]=pl[1][0]-pl[0][0];
          vproj[1]=pl[1][1]-pl[0][1];
          vproj[2]=pl[1][2]-pl[0][2];
          lp0p1=v_betrag( vproj );
          v_norm( vproj, eg );
          /*
          printf("n1: %lf %lf %lf\n",pl[0][0],pl[0][1],pl[0][2]);
          printf("n2: %lf %lf %lf\n",pl[1][0],pl[1][1],pl[1][2]);
          printf("v: %lf %lf %lf\n",vproj[0],vproj[1],vproj[2]);
          printf("pt1: %lf %lf %lf\n",pt1[0],pt1[1],pt1[2] );
          printf("pt2: %lf %lf %lf\n",pt2[0],pt2[1],pt2[2] );
          printf("pt3: %lf %lf %lf\n",pt3[0],pt3[1],pt3[2] );
          */

          /* determine the center of the edge */
          vn_av[0]=(pl[1][0]+pl[0][0])*.5;
          vn_av[1]=(pl[1][1]+pl[0][1])*.5;
          vn_av[2]=(pl[1][2]+pl[0][2])*.5;
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, pl[0][0],pl[0][1],pl[0][2],
              sum_tri, &near_node[0], n_closest_nodes/3);
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, vn_av[0],vn_av[1],vn_av[2],
              sum_tri, &near_node[n_closest_nodes/3], n_closest_nodes/3);
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, pl[1][0],pl[1][1],pl[1][2],
              sum_tri, &near_node[n_closest_nodes/3*2], n_closest_nodes-n_closest_nodes/3*2);

          /* go over the closest splitting-tri */
          for( nn=0; nn<n_closest_nodes; nn++)
          {
            n=near_node[nn];
            v_result( tri[n].p1, tri[n].p2, p01 );
            v_norm( p01, eu );
            v_result( tri[n].p1, tri[n].p3, p01 );
            v_norm( p01, ev );
            v_prod( eu, ev, p01);
            v_norm( p01, en );
            /* bestimme den Abstand zwischen den Aufpunkten der Linie und Ebene  */
            v_result( pl[0], tri[n].p1, p01 );
            g = AsplitL( p01, eu, ev, eg, en );
            v_scal( &g, eg, p01 );
            v_add( pl[0], p01, &psplt[0][0] );
            // projpl[0] and pl[1] onto en and generate new points there
            v_result( &psplt[0][0], pl[0], p01 );
            g=v_sprod(p01,en);
            v_scal(&g,en, p01);
            v_add(&psplt[0][0],p01,pp1);
            v_result( &psplt[0][0], pl[1], p01 );
            g=v_sprod(p01,en);
            v_scal(&g,en, p01);
            v_add(&psplt[0][0],p01,pp2);
  
            dist=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pp1, en, 1., &orient);
            dist2=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pp2, en, 1., &orient);
    
            /* check if nodes are positioned in (or close to) the splitting plane, these are special cases */
            //printf(" n1:%d dist:%lf ori:%d \n", n1, dist, orient);
                //printf(" n2:%d dist:%lf ori:%d \n", n2, dist2, orient);
            
            if ((abs(dist)<=ltol)||(abs(dist2)<=ltol))
            {
              if(param[0]=='e') { seta(set_eorig, "e", e); goto next_elem; }
              /* was this collapsed node evaluated? */
                     /* which side is closer to n1 n2? */
              if (abs(dist)<ltol) ns= n1; 
              else ns= n2;
              for(ii=0; ii<c; ii++) if(ncollapsed[ii]== ns) break;
              if(ii< c) continue;

              ncollapsed[c]=ns;

              //debug_pre_split printf(" node in splitting plane:%d sum_collapsed:%d\n",ncollapsed[c], c+1);

              /* remember the normal vector of the splitting triangle */
              v_result( tri[n].p1,tri[n].p2, v12);
              v_result( tri[n].p1,tri[n].p3, v13);
              v_prod(v12,v13,vn);
              v_norm(vn,&vn_tri[m][0]);

              /* The split-point is just a copy of one end-point of that line */

              /* check if the ns exists already */
              ns=-1;
              for(ii=0; ii<n1ns[n1].sum; ii++) if(n1ns[n1].n2[ii]==n1) { ns=n1ns[n1].nm[ii]; nc=n1ns[n1].nc[ii]; }
              for(ii=0; ii<n1ns[n2].sum; ii++) if(n1ns[n2].n2[ii]==n2) { ns=n1ns[n2].nm[ii]; nc=n1ns[n2].nc[ii]; }
                                    
              if(ns==-1)
              {
                /* move n1 or n2 to the splitting plane */
                /* determine normal direction to plane (before the loop starts, common for all elems (TBD)) */
                distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pl[0], vn, 100.0, &orient);
                if (abs(distn)<ltol)
                {
                  node[n1].nx=pl[0][0]+vn_tri[m][0]*distn;
                  node[n1].ny=pl[0][1]+vn_tri[m][1]*distn;
                  node[n1].nz=pl[0][2]+vn_tri[m][2]*distn;
                }
                distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pl[1], vn, 100.0, &orient);
                if (abs(distn)<ltol)
                {
                  node[n2].nx=pl[1][0]+vn_tri[m][0]*distn;
                  node[n2].ny=pl[1][1]+vn_tri[m][1]*distn;
                  node[n2].nz=pl[1][2]+vn_tri[m][2]*distn;
                      }

                /* which side is closer to n1 n2? */
                if (abs(dist)<ltol) ns= n1; 
                else ns= n2;

                /* copy */
                nc=nod( anz, &node, 1, anz->nnext++, node[ns].nx, node[ns].ny, node[ns].nz, 0);

                //debug_pre_split printf(" nc:%d distn:%lf ori:%d n:%f %f %f\n", node[nc].nr, distn, orient, vn[0], vn[1], vn[2]);
                if(e_enqire[e].type==6)
                {
                  if ( (n1nm = (N1nm *)realloc(n1nm, (anz->nmax+1) * sizeof(N1nm))) == NULL )
                  { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].sum=0;
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].n2=n1nm[ii].nm=n1nm[ii].nc=NULL;
                }
                if ( (n1ns[ns].n2 = (int *)realloc( n1ns[ns].n2, (n1ns[ns].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[ns].nm = (int *)realloc( n1ns[ns].nm, (n1ns[ns].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[ns].nc = (int *)realloc( n1ns[ns].nc, (n1ns[ns].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                //if(ns==n1) n1ns[ns].n2[n1ns[ns].sum]=n2; else n1ns[ns].n2[n1ns[ns].sum]=n1;
                n1ns[ns].n2[n1ns[ns].sum]=ns;
                n1ns[ns].nm[n1ns[ns].sum]=node[ns].indx;
                n1ns[ns].nc[n1ns[ns].sum]=nc;
                n1ns[ns].sum++;
                nnew[m]=node[ns].indx;
              }
              else nnew[m]=ns;
              ncpy[m]=nc;

              /* assign the corner-nodes */
              nref[m][0]=edge[k++]=n1;
              nref[m][1]=edge[k++]=n2;

              //debug_pre_split printf("m:%d c:%d n1:%d n2:%d node:%d copy:%d at  %lf %lf %lf\n", m+1, c+1, n1,n2, node[nnew[m]].nr, node[ncpy[m]].nr,  node[ns].nx, node[ns].ny, node[ns].nz );
              m++;
              c++;
              goto nextEdge;
            }

            /* create a node only if the vector goes through the tri and if the sign of dist has changed (scip the first line-vector (p==3))*/
            else if (( dist != OUTSIDE ) && ( dist2 != OUTSIDE ) && (dist2*dist <= 0.))
            {
              if(param[0]=='e') { seta(set_eorig, "e", e); goto next_elem; }

              /* remember the normal vector of the splitting triangle (to be replaced by "orient") */
              v_result( tri[n].p1,tri[n].p2, v12);
              v_result( tri[n].p1,tri[n].p3, v13);
              v_prod(v12,v13,vn);
              v_norm(vn,&vn_tri[m][0]);

              /* check if the ns exists already */
              ns=-1;
              for(ii=0; ii<n1ns[n1].sum; ii++) if(n1ns[n1].n2[ii]==n2) { ns=n1ns[n1].nm[ii]; nc=n1ns[n1].nc[ii]; }
              for(ii=0; ii<n1ns[n2].sum; ii++) if(n1ns[n2].n2[ii]==n1) { ns=n1ns[n2].nm[ii]; nc=n1ns[n2].nc[ii]; }
	  	
              /* create a split-point and a copy if not existent */
              if(ns==-1)
	      {
                v_result( pl[0], &psplt[0][0], vs );
                lp0p1=v_betrag( vs );
                lps+=lp0p1;
    
    	        ns=nod( anz, &node, 1, anz->nnext++,  psplt[0][0], psplt[0][1], psplt[0][2], 0);
    	        nc=nod( anz, &node, 1, anz->nnext++,  psplt[0][0], psplt[0][1], psplt[0][2], 0);

                if(e_enqire[e].type==6)
	        {
                  if ( (n1nm = (N1nm *)realloc(n1nm, (anz->nmax+1) * sizeof(N1nm))) == NULL )
                  { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].sum=0;
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].n2=n1nm[ii].nm=n1nm[ii].nc=NULL;
		}
                if ( (n1ns[n1].n2 = (int *)realloc( n1ns[n1].n2, (n1ns[n1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[n1].nm = (int *)realloc( n1ns[n1].nm, (n1ns[n1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[n1].nc = (int *)realloc( n1ns[n1].nc, (n1ns[n1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1ns[n1].n2[n1ns[n1].sum]=n2;
                n1ns[n1].nm[n1ns[n1].sum]=ns;
                n1ns[n1].nc[n1ns[n1].sum]=nc;
                n1ns[n1].sum++;
	      }
              nnew[m]=ns;
              ncpy[m]=nc;

              /* assign the corner-nodes */
              nref[m][0]=edge[k++]=n1;
              nref[m][1]=edge[k++]=n2;
    
              //debug_pre_split printf("m:%d n1:%d n2:%d split node:%d copy:%d   %lf %lf %lf\n", m+1, n1,n2, node[nnew[m]].nr,  node[ncpy[m]].nr, psplt[0][0], psplt[0][1], psplt[0][2] );
              m++;
	      goto nextEdge;
            }
          }

          { nunspl[l][0]=n1; nunspl[l++][1]=n2; }
          //debug_pre_split printf("l:%d unsplit nodes:%d %d\n",l, n1,n2 );
 nextEdge:;
        }

        if((e_enqire[e].type==6)&&(m>=2))
        {
          /* remember the corner-nodes of the midside nodes of the original element */
          j=0;
          for(i=0; i<6; i++)
          {
            n1=e_enqire[e].nod[nodseq_te10[j++]];
            nm=node[e_enqire[e].nod[nodseq_te10[j++]]].indx;
            n2=e_enqire[e].nod[nodseq_te10[j++]];
	    //debug_pre_split printf("j:%d n1:%d n2:%d nm:%d\n", j, n1,n2, nm);

            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nc = (int *)realloc( n1nm[n1].nc, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].nc[n1nm[n1].sum]=-1;
            n1nm[n1].sum++;
	  }
        }

        /* *******  create new elems ********** */

        if((m==3)&&(c==2)) /* (cut along edge through tet => 2new tets) */
	{
          //debug_pre_split printf("e:%d cut along edge through tet => 2new tets\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* copy element, modify later */
          if(e_enqire[e].type==3)
	  {
            elem_define(anz,&e_enqire, anz->enext++, 3, e_enqire[e].nod, 1, 0 );
	  }
          else
	  {
            elem_define(anz,&e_enqire, anz->enext++, 6, e_enqire[e].nod, 1, 0 );
	  }

          /* search the tetra-top-nodes */
          for(i=0; i<4; i++)
	  {
            nbuf[i]=e_enqire[e].nod[i];
            for(j=0; j<m; j++) if(e_enqire[e].nod[i]==node[nnew[j]].nr) nbuf[i]=-1;
            //debug_pre_split for(j=0; j<m; j++) printf("en:%d nnew %d nbuf %d\n", e_enqire[e].nod[i], node[nnew[j]].nr, nbuf[i]);
	  }
          j=0;
          for(i=0; i<4; i++)
	  {
            if(nbuf[i]!=-1) nbuf[j++]=nbuf[i];
	  }
          //debug_pre_split printf("j:%d top-nodes: %d %d\n", j, nbuf[0], nbuf[1]);
          if(j>2)
          {
            printf("found 3. top-node:%d\n", nbuf[1]);
            for(i=0; i<4; i++) printf("en:%d %d\n", e_enqire[e].nod[i], nbuf[i]);
            exit(0);
          }

          /* prepare the elements */
          /* 1st element gets copied nodes ncpy, oriented in the same way as the plane */
          for(i=0; i<3; i++)
	  {
            seta(set_ntop, "n",node[ncpy[i]].nr);
            seta(set_nbot, "n",node[nnew[i]].nr);
	  }
          seta(set_etop, "e", e);
          seta(set_ebot, "e", anz->emax);

          /* check the orientation of nnew */
          v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[1]].nr].nx, v12);
          v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[2]].nr].nx, v13);
          v_result( &node[node[nnew[0]].nr].nx, &node[edge[i]].nx, v15);
          v_prod(v12,v13,vn);
          v_norm(vn,vn);
          if (v_sprod(vn_av,vn) > 0.)
	  {
            ss=1;
            j=3;
            for(i=0; i<3; i++)
	    {
              e_enqire[e].nod[i]=node[ncpy[i]].nr;
              e_enqire[anz->emax].nod[i]=node[nnew[--j]].nr;
	    }
	  }
	  else
	  {
            ss=0;
            j=3;
            for(i=0; i<3; i++)
	    {
              e_enqire[e].nod[i]=node[ncpy[--j]].nr;
              e_enqire[anz->emax].nod[i]=node[nnew[i]].nr;
	    }
	  }

          /* 1st element has copied nodes and nbuf on the positive side of the plane */
          v_result( &node[node[nnew[0]].nr].nx,&node[nbuf[0]].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
              e_enqire[e].nod[3]=nbuf[0];
              e_enqire[anz->emax].nod[3]=nbuf[1];
	  }
          else
          {
              e_enqire[e].nod[3]=nbuf[1];
              e_enqire[anz->emax].nod[3]=nbuf[0];
	  }

          /* change the midnodes */
          if(e_enqire[e].type==6)
          {
            /* first for the copied elem anz->emax (this has the orig-nodes)  and only the basis (nnew,ncpy) */

            /* gen mitside-nodes */
            for (n=0; n<3; n++)
            {
              nn1= e_enqire[anz->emax].nod[nodseq_te4[n*2]];
              nn2= e_enqire[anz->emax].nod[nodseq_te4[n*2+1]];
              //debug_pre_split printf("e:%d n12:%d %d \n", e, nn1,nn2 );
        
              /* check if the nm and nc exist already */
              nm=nc=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; nc=n1nm[nn1].nc[i]; }
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; nc=n1nm[nn2].nc[i]; }
        
              if(nc==-1)
              {
                if(nm==-1)
                {
                  /* generate new node */
                  nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
                  node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                  node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                  node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                  if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                  { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                  if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                  { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                  n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                  n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                  n1nm[nn1].sum++;
                }        
                /* generate new copy node */
                nc=nod( anz, &node, 1, anz->nnext++, node[node[nm].nr].nx,node[node[nm].nr].ny,node[node[nm].nr].nz, 0 );
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].nc[n1nm[nn1].sum]=nc;
              }        
              e_enqire[anz->emax].nod[nodseq_te10[n*3+1]]=node[nm].nr;
              seta(set_nbot, "n",node[nm].nr);
              //debug_pre_split printf("e:%d p:%d n12:%d %d nm:%d\n", anz->emax, nodseq_te10[n*3+1], nn1,nn2, node[nm].nr );
              switch(n)
		{ case 0: jj=5; break;  case 1: jj=4; break; case 2: jj=6; break; default: printf("ERROR: n:%d not possible\n",n); exit(0); }
              e_enqire[e].nod[jj]=node[nc].nr;
              seta(set_ntop, "n",node[nc].nr);
              //debug_pre_split printf("e:%d p:%d n12:%d %d nc:%d\n", e, jj, nn1,nn2, node[nc].nr );
            }

            /* edges running from base to top */
            ec=e;
            for (ee=0; ee<2; ee++)
	    {
	     if(ee==1) ec=anz->emax;
             for (n=0; n<3; n++)
             {
            /* collapsed nodes require a first check with the original nodes */
              nn1= node[nnew[n]].nr;
              nn2= e_enqire[ec].nod[nodseq_te4[(n+3)*2+1]];
              if(ee==0)
              {
                if((nn1!=ncollapsed[0])&&(nn1!=ncollapsed[1])) nn1= node[ncpy[n]].nr;
              }
        
              /* check if the nm exist already (nc never used here because its not splittet running to TOP) */
              nm=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
        
              if(nm==-1)
              {
                /* generate new node */
                nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
                node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                n1nm[nn1].sum++;
              }        
              if(ee==ss) e_enqire[ec].nod[9-n]=node[nm].nr;
              else e_enqire[ec].nod[n+7]=node[nm].nr;
              //debug_pre_split printf("e:%d p:%d n12:%d %d nm:%d\n", ec, n+7, nn1,nn2, node[nm].nr );
             }        
	    }
	  }
	}
        else if((m==1)&&(c==1)) /* 1 node copied (cut at top) */
	{
          //debug_pre_split printf("e:%d cut at top\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* sort the elements according to their orientation regarding the splitting triangle */
          /* the element on the fronside of the plane gets the copied nodes */
          if(node[nnew[0]].nr==nref[0][0]) n=nref[0][1]; else n=nref[0][0];
          v_result( &node[node[nnew[0]].nr].nx,&node[n].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
            /* redefine the tet */
            for(i=0; i<4; i++)
              for(ii=0; ii<m; ii++)
                if(e_enqire[e].nod[i]==node[nnew[ii]].nr)
		{
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i],node[ncpy[ii]].nr );
	          e_enqire[e].nod[i]=node[ncpy[ii]].nr;
                }
            seta(set_ntop, "n",node[ncpy[0]].nr);
            seta(set_etop, "e", e);
          }
          else
          {
            seta(set_nbot, "n",node[nnew[0]].nr);
            seta(set_ebot, "e", e);
          }
	}
        else if((m==3)&&(c==3)) /* 3 nodes copied (cut at basis) */
	{
          //debug_pre_split printf("e:%d cut at basis\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* sort the elements according to their orientation regarding the splitting triangle */
          /* the element on the fronside of the plane gets the copied nodes */
          if(node[nnew[0]].nr==nref[0][0]) n=nref[0][1]; else n=nref[0][0];
          //printf("topn:%d bottomn;%d\n", n, node[nnew[0]].nr);
          v_result( &node[node[nnew[0]].nr].nx,&node[n].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
            /* redefine the tet */
            for(i=0; i<4; i++)
              for(ii=0; ii<m; ii++)
                if(e_enqire[e].nod[i]==node[nnew[ii]].nr)
		{
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i],node[ncpy[ii]].nr );
	          e_enqire[e].nod[i]=node[ncpy[ii]].nr;
                }

            /* copy also the midside nodes. */
	    if(e_enqire[e].type==6)
            {
              /* copy the midside nodes */
              jj=0;
              for(ii=0; ii<m; ii++)
  	      {
                nn1=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
                nn2=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
		//debug_pre_split printf("nn1:%d nn2:%d\n", nn1,nn2);

                /* check if the nc exists already */
                nc=nm=s=-1;
                for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; nc=n1nm[nn1].nc[i]; n=nn1; s=i; break; }
  	        if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; nc=n1nm[nn2].nc[i];  n=nn2; s=i; break; } } }
		//debug_pre_split printf("nc:%d nm:%d\n", nc, nm);
    
                if(nm==-1)
                { printf("\n\n ERROR in midnode not found\n\n") ; exit(-1); }    
                if(nc==-1)
                {
                  /* generate new mid-node */
                  nc=nod( anz, &node, 1, anz->nnext++, node[node[nm].nr].nx, node[node[nm].nr].ny, node[node[nm].nr].nz, 0 );
                  n1nm[n].nc[s]=nc;
                }

		//debug_pre_split printf("nc:%d nod:%d\n", nc,node[nc].nr);
                /* redefine the tet */
                for(i=4; i<10; i++) if(e_enqire[e].nod[i]==node[nm].nr)
                {
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i], node[nc].nr );
                  e_enqire[e].nod[i]=node[nc].nr;
                  break;
                }
                seta(set_ntop, "n",node[nm].nr);
              }
	    }
            for(ii=0; ii<3; ii++) seta(set_ntop, "n",node[ncpy[ii]].nr);
            seta(set_etop, "e",e);
          }
	  else
	  {
	    if(e_enqire[e].type==6)
            {
              /* copy the midside nodes */
              jj=0;
              for(ii=0; ii<m; ii++)
  	      {
                nn1=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
                nn2=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
		//debug_pre_split printf("nn1:%d nn2:%d\n", nn1,nn2);

                nm=s=-1;
                for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; s=i; break; }
    	        if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; break; } } }
  	        //debug_pre_split printf("nm:%d\n", nm);
                seta(set_nbot, "n",node[nm].nr);
              }
	    }
            for(ii=0; ii<3; ii++) seta(set_nbot, "n",node[nnew[ii]].nr);
            seta(set_ebot, "e", e);
          }

	}
        else if((m==2)&&(c==2)) /* 2 node copied (cut along an edge) */
	{
          //debug_pre_split printf("e:%d cut along an edge\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* sort the elements according to their orientation regarding the splitting triangle */
          /* the element on the fronside of the plane gets the copied nodes */
          n=-1; 
          for(i=0; i<4; i++) if((node[nnew[0]].nr!=e_enqire[e].nod[i])&&(node[nnew[1]].nr!=e_enqire[e].nod[i])) { n=e_enqire[e].nod[i]; break; }
          if(n==-1)
	  { printf("ERROR\n"); exit(0); }
          //debug_pre_split printf("topn:%d bottomn;%d\n", n, node[nnew[0]].nr);


          v_result( &node[node[nnew[0]].nr].nx,&node[n].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
            /* redefine the tet */
            for(jj=0; jj<4; jj++)
              for(ii=0; ii<m; ii++)
                if(e_enqire[e].nod[jj]==node[nnew[ii]].nr)
	        {
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[jj], node[ncpy[ii]].nr );
                  e_enqire[e].nod[jj]=node[ncpy[ii]].nr;
		}

            /* copy also the midside nodes. */
	    if(e_enqire[e].type==6)
            {
              /* copy the midside nodes */
              jj=0;
              for(ii=0; ii<m; ii++)
  	      {
                nn1=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
                nn2=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
		//debug_pre_split printf("nn1:%d nn2:%d\n", nn1,nn2);

                /* check if the nc exists already */
                nc=nm=s=-1;
		//debug_pre_split printf("nn1sum:%d nn2sum:%d\n", n1nm[nn1].sum,n1nm[nn2].sum);
                for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; nc=n1nm[nn1].nc[i]; n=nn1; s=i; break; }
  	        if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; nc=n1nm[nn2].nc[i];  n=nn2; s=i; break; } } }
		//debug_pre_split printf("nc:%d nm:%d\n", nc, nm);
    
                if(nm==-1)
                { printf("\n\n ERROR in midnode not found\n\n") ; exit(-1); }    
                if(nc==-1)
                {
                  /* generate new mid-node */
                  nc=nod( anz, &node, 1, anz->nnext++, node[node[nm].nr].nx, node[node[nm].nr].ny, node[node[nm].nr].nz, 0 );
                  n1nm[n].nc[s]=nc;
                }

		//debug_pre_split printf("nc:%d nod:%d\n", nc,node[nc].nr);
                /* redefine the tet */
                for(i=4; i<10; i++)
                  if(e_enqire[e].nod[i]==node[nm].nr)
                  {
                    //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i], node[nc].nr );
                    e_enqire[e].nod[i]=node[nc].nr;
                    break;
                  }
              }
              seta(set_ntop, "n",node[nc].nr);
	    }
            seta(set_ntop, "n",node[ncpy[0]].nr);
            seta(set_ntop, "n",node[ncpy[1]].nr);
            seta(set_etop, "e", e);
          }
	  else
	  {
            nn1=node[nnew[0]].nr;
            nn2=node[nnew[1]].nr;
	    if(e_enqire[e].type==6)
            {
              nm=s=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; s=i; break; }
    	      if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; break; } } }
              seta(set_nbot, "n",node[nm].nr);
            }
            seta(set_nbot, "n",nn1);
            seta(set_nbot, "n",nn2);
            seta(set_ebot, "e", e);
          }

	}
        else if(m==4)  /* 4 edges splitted: 2 pentas */
        {
          //debug_pre_split printf("4 edges splitted: 2 pentas\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* the two unsplitted lines must be unconnected */
          if((nunspl[0][0]==nunspl[1][0])||(nunspl[0][0]==nunspl[1][1])) goto next_elem; 
          if((nunspl[0][1]==nunspl[1][0])||(nunspl[0][1]==nunspl[1][1])) goto next_elem; 

          /* sort the elements according to their orientation regarding the splitting triangle */
          v_result( &node[nunspl[0][0]].nx,&node[nunspl[1][0]].nx, v15);
          v_result( &node[nunspl[0][1]].nx,&node[nunspl[1][1]].nx, v13);
          v_norm(v15,v15);
          v_norm(v13,v13);
          v_add(v15,v13,v12);
          if (v_sprod(vn_av,v12) < 0.) { n1=1; n2=0; } else { n1=0; n2=1; }
 
          /* 1. penta */
          d1=1; d2=4;
          nel[0]=nunspl[n1][0];
          nel[3]=nunspl[n1][1];
          for(i=0; i<4; i++)
          {
            for(j=0; j<2; j++)
            {
	      //printf("ref:%d unspl:%d %d\n", nref[i][j], nunspl[n1][0],nunspl[n1][1]);
	      if(nref[i][j]==nunspl[n1][0])  nel[d1++]=node[nnew[i]].nr;
	      if(nref[i][j]==nunspl[n1][1])  nel[d2++]=node[nnew[i]].nr;
	    }
	  }

          /* check the orientation */
          v_result( &node[nel[0]].nx, &node[nel[1]].nx, v12);
          v_result( &node[nel[0]].nx, &node[nel[2]].nx, v13);
          v_result( &node[nel[0]].nx, &node[nunspl[n1][1]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n > bgrenz) { ibuf=nel[1]; nel[1]=nel[2]; nel[2]=ibuf ; }

          /* check the orientation */
          v_result( &node[nel[3]].nx, &node[nel[4]].nx, v12);
          v_result( &node[nel[3]].nx, &node[nel[5]].nx, v13);
          v_result( &node[nel[3]].nx, &node[nunspl[n1][0]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n < bgrenz) { ibuf=nel[4]; nel[4]=nel[5]; nel[5]=ibuf ; }

          if(e_enqire[e].type==6)
	  {
            /* gen mitside-nodes */
            for (n=0; n<6; n++) nbuf[n]=nel[n];

            for (n=0; n<9; n++)
            {
              nel[nodseq_pe15[n*3]]=  nn1= nbuf[nodseq_pe6[n*2]];
              nel[nodseq_pe15[n*3+2]]=   nn2= nbuf[nodseq_pe6[n*2+1]];

              /* check if the nm exists already */
              nm=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
  
              if(nm==-1)
              {
                /* generate new node */
                nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
                node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                n1nm[nn1].sum++;
              }

              nel[nodseq_pe15[n*3+1]]=node[nm].nr;
            }
            elem_define(anz,&e_enqire, e, 5, nel, 1, 0 );
            seta(set_nbot, "n",nel[1]);
            seta(set_nbot, "n",nel[2]);
            seta(set_nbot, "n",nel[4]);
            seta(set_nbot, "n",nel[5]);
            seta(set_nbot, "n",nel[7]);
            seta(set_nbot, "n",nel[10]);
            seta(set_nbot, "n",nel[11]);
            seta(set_nbot, "n",nel[13]);
	  }
          else
          {
            elem_define(anz,&e_enqire, e, 2, nel, 1, 0 );
            seta(set_nbot, "n",nel[1]);
            seta(set_nbot, "n",nel[2]);
            seta(set_nbot, "n",nel[4]);
            seta(set_nbot, "n",nel[5]);
          }
          seta(set_ebot, "e",e);

          /* 2. penta, redefine tet */
          d1=1; d2=4;
          for(i=0; i<4; i++)
          {
            nel[0]=nunspl[n2][0];
            nel[3]=nunspl[n2][1];
            for(j=0; j<2; j++)
            {
	      //printf("ref:%d unspl:%d %d\n", nref[i][j], nunspl[n2][0],nunspl[n2][1]);
	      if(nref[i][j]==nunspl[n2][0])  nel[d1++]=node[ncpy[i]].nr;
	      if(nref[i][j]==nunspl[n2][1])  nel[d2++]=node[ncpy[i]].nr;
	    }
	  }
	  //for(i=0; i<6; i++) printf("%d ", nel[i]); printf("\n");

          /* check the orientation */
          v_result( &node[nel[0]].nx, &node[nel[1]].nx, v12);
          v_result( &node[nel[0]].nx, &node[nel[2]].nx, v13);
          v_result( &node[nel[0]].nx, &node[nunspl[n2][1]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n > bgrenz) { ibuf=nel[1]; nel[1]=nel[2]; nel[2]=ibuf ; }

          /* check the orientation */
          v_result( &node[nel[3]].nx, &node[nel[4]].nx, v12);
          v_result( &node[nel[3]].nx, &node[nel[5]].nx, v13);
          v_result( &node[nel[3]].nx, &node[nunspl[n2][0]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n < bgrenz) { ibuf=nel[4]; nel[4]=nel[5]; nel[5]=ibuf ; }

          if(e_enqire[e].type==5)
	  {
            /* gen mitside-nodes */
            for (n=0; n<6; n++) nbuf[n]=nel[n];

            for (n=0; n<9; n++)
            {
              nel[nodseq_pe15[n*3]]=  nn1= nbuf[nodseq_pe6[n*2]];
              nel[nodseq_pe15[n*3+2]]=   nn2= nbuf[nodseq_pe6[n*2+1]];
 
              /* check if the nm exists already */
              nm=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
  
              if(nm==-1)
              {
                /* generate new node */
                nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
                node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                n1nm[nn1].sum++;
              }

              nel[nodseq_pe15[n*3+1]]=node[nm].nr;
            }
            elem_define(anz,&e_enqire, anz->enext++, 5, nel, 1, 0 );
            seta(set_ntop, "n",nel[1]);
            seta(set_ntop, "n",nel[2]);
            seta(set_ntop, "n",nel[4]);
            seta(set_ntop, "n",nel[5]);
            seta(set_ntop, "n",nel[7]);
            seta(set_ntop, "n",nel[10]);
            seta(set_ntop, "n",nel[11]);
            seta(set_ntop, "n",nel[13]);
	  }
          else
          {
            elem_define(anz,&e_enqire, anz->enext++, 2, nel, 1, 0 );
            seta(set_ntop, "n",nel[1]);
            seta(set_ntop, "n",nel[2]);
            seta(set_ntop, "n",nel[4]);
            seta(set_ntop, "n",nel[5]);
          }
          seta(set_etop, "e", anz->emax);
	}
	else if(m==3)
        {
	  //debug_pre_split printf("c:%d e:%d cut at top through tet => 1tet 1pe\n",c, e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* search common node of splitted edges, this will be the tip-node of the resulting tet */
          /* exclude all nodes on collapsed edges */
          /* and later replace also nref with nodes from the edge running from ncollapsed to the tip (at n) */
          for(n=0; n<m; n++)
          {
            if((nref[n][0]==ncollapsed[0])||(nref[n][1]==ncollapsed[0])) { edge[n*2]=edge[n*2+1]=0; break; }
	  } 
          qsort( edge, 6, sizeof(int), (void *)compareInt );
          l=0;
          for(i=0;i<5; i++)
          {
            if(edge[i]==edge[i+1]) l++; else l=0;
	    //debug_pre_split printf("node[%d]:%d l:%d\n",i,edge[i],l);
            if((l==1)&&(edge[i]))
            {
              /* tip of tet was found */
              if(c) { nref[n][0]=ncollapsed[0]; nref[n][1]=edge[i]; }
              /* sort the elements according to their orientation regarding the splitting triangle */
              //v_result( &node[node[nnew[0]].nr].nx,&node[edge[i]].nx, v12);
              //if (v_sprod(vn_av,v12) < 0.)
              v_result( &node[node[nnew[0]].nr].nx,&node[edge[i]].nx, v15);
              v_result( &node[node[nnew[1]].nr].nx,&node[edge[i]].nx, v13);
              v_result( &node[node[nnew[2]].nr].nx,&node[edge[i]].nx, v12);
              v_norm(v15,v15);
              v_norm(v13,v13);
              v_norm(v12,v12);
              v_add(v15,v13,vn);
              v_add(vn,v12,v15);

              if (v_sprod(vn_av,v15) < 0.)
              {
                n1=0;
              }
              else
              {
                n1=1;
	      }
	      

              /* check the orientation */
              v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[1]].nr].nx, v12);
              v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[2]].nr].nx, v13);
              v_result( &node[node[nnew[0]].nr].nx, &node[edge[i]].nx, v15);
              v_prod(v12,v13,vn);
              v_result(v15,vn,v15n);
              bvn=v_betrag(vn);
              bv15=v_betrag(v15);
              bgrenz=sqrt(bvn*bvn+bv15*bv15);
              bv15n=v_betrag(v15n);

	      /*	      
          printf("ntip:%d n:%d\n", edge[i],node[nnew[0]].nr);
	  printf("tri: %f %f %f %f\n", vn_tri[0][0], vn_tri[0][1], vn_tri[0][2], v_betrag(vn_av));
	  printf("v12: %f %f %f %f\n", v12[0], v12[1], v12[2], v_betrag(v12));
	  printf("v15: %f %f %f %f\n", v15[0], v15[1], v15[2], bv15);
              printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
              printf("common node:%d\n",edge[i+1]);
	      */

              if (bv15n < bgrenz)
              {
                /* nodes for the penta elem */
                for(j=0; j<3; j++) 
                {
		  if(n1) nel[j+3]=node[nnew[j]].nr;
		  else   nel[j+3]=node[ncpy[j]].nr;
                  if( edge[i]==nref[j][0]) nel[j]=nref[j][1]; else nel[j]=nref[j][0];

                  /* for collapsed penta */
                  if(!n1) if(c) if(nel[j]==ncollapsed[0]) ibuf=nel[j]=node[ncpy[j]].nr; 
                }
                /* redefine the tet */
                m=3; j=0;
                if(n1) while(j<3) e_enqire[e].nod[m--]=node[ncpy[j++]].nr;
                else   while(j<3) e_enqire[e].nod[m--]=node[nnew[j++]].nr;

                /* for collapsed penta on the tet-side */
                if(n1) if(c) for(j=0; j<3; j++) if(node[nnew[j]].nr==ncollapsed[0]) { ibuf=node[ncpy[j]].nr; break; }
              }         
              else
              {
                /* nodes for the penta elem */
                for(j=0; j<3; j++) 
                {
                  if(n1) nel[j]=node[nnew[j]].nr;
                  else   nel[j]=node[ncpy[j]].nr;
                  if( edge[i]==nref[j][0]) nel[j+3]=nref[j][1]; else nel[j+3]=nref[j][0];

                  /* for collapsed penta */
                  if(!n1) if(c) if(nel[j+3]==ncollapsed[0])  ibuf=nel[j+3]=node[ncpy[j]].nr;
                }
                /* redefine the tet */
                m=1; j=0;
                if(n1)  while(j<3) e_enqire[e].nod[m++]=node[ncpy[j++]].nr;
                else    while(j<3) e_enqire[e].nod[m++]=node[nnew[j++]].nr;

                /* for collapsed penta on the tet-side */
                if(n1) if(c) for(j=0; j<3; j++) if(node[nnew[j]].nr==ncollapsed[0]) { ibuf=node[ncpy[j]].nr; break; }
              }
              e_enqire[e].nod[0]=edge[i];

              if(e_enqire[e].type==6)
	      {
                /* gen mitside-nodes */
                for (n=0; n<6; n++)
                {
                  nn1= e_enqire[e].nod[nodseq_te4[n*2]];
                  nn2= e_enqire[e].nod[nodseq_te4[n*2+1]];

                  /* for collapsed penta on the tet-side */
                  if((n1)&&(c)&&((n<4)&&(n!=1)))
                  {
                    if(nn1==ibuf) nn1=ncollapsed[0];
                    if(nn2==ibuf) nn2=ncollapsed[0];
                  }
                  //debug_pre_split printf("b n1:%d c:%d tet:%d n12 %d %d col %d ibuf %d\n", n1, c, e, nn1, nn2, ncollapsed[0], ibuf);
        
                  /* check if the nm exists already */
                  nm=-1;
                  for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
                  for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
        
                  if(nm==-1)
                  {
                    /* generate new node */
                    nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
                    node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                    node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                    node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                    if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                    { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                    if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                    { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                    if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                    { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                    n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                    n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                    n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                    n1nm[nn1].sum++;
                  }        
                  e_enqire[e].nod[nodseq_te10[n*3+1]]=node[nm].nr;
                  if((n>3)||(n==1))
                  {
                    if(!n1) seta(set_nbot, "n",node[nm].nr);
                    else   seta(set_ntop, "n",node[nm].nr);
                  }
                }

                /* penta */

                for (n=0; n<6; n++) nbuf[n]=nel[n];
                for (n=0; n<9; n++)
                {
                  nel[nodseq_pe15[n*3]]=  nn1= nbuf[nodseq_pe6[n*2]];
                  nel[nodseq_pe15[n*3+2]]=   nn2= nbuf[nodseq_pe6[n*2+1]];

                  /* for collapsed penta */
                  if(nn1==nn2)
		  {
                    nel[nodseq_pe15[n*3+1]]=nn1;
		  }
                  else
		  {
                    /* for collapsed penta */
                    //debug_pre_split printf("n1:%d c:%d ibuf:%d col:%d n12 %d %d\n", n1,c, ibuf, ncollapsed[0], nn1, nn2);
                    if(!n1) if(c) { if(nn1==ibuf) nn1=ncollapsed[0]; if(nn2==ibuf) nn2=ncollapsed[0]; }

                    /* check if the nm exists already */
                    nm=-1;
                    for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
                    for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
        
                    if(nm==-1)
                    {
                      /* generate new node */
                      nm=nod( anz, &node, 1, anz->nnext++, 0., 0., 0., 0 );
                      node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                      node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                      node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                      if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                      { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                      if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                      { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                      if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                      { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                      n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                      n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                      n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                      n1nm[nn1].sum++;
                    }
                    nel[nodseq_pe15[n*3+1]]=node[nm].nr;
                  
                    if (bv15n < bgrenz)
                    {
                    if((n>2)&&(n<6))
                    {
                        if(n1) seta(set_nbot, "n",node[nm].nr);
                        else   seta(set_ntop, "n",node[nm].nr);
                    }
                  }
                    else
                    {
                    if(n<3)
                    {
                        if(n1) seta(set_nbot, "n",node[nm].nr);
                        else   seta(set_ntop, "n",node[nm].nr);
                    }
                  }
                  
                  }
                }
                elem_define(anz,&e_enqire, anz->enext++, 5, nel, 1, 0 );
	      }
	      else
              {
                elem_define(anz,&e_enqire, anz->enext++, 2, nel, 1, 0 );
              }

              for(j=0; j<3; j++) { seta(set_ntop, "n",node[ncpy[j]].nr); seta(set_nbot, "n",node[nnew[j]].nr); }
              if (n1==0)
              {
                seta(set_ebot, "e", e);
                seta(set_etop, "e",anz->emax);
              }
              else
              {
                seta(set_etop, "e", e);
                seta(set_ebot, "e",anz->emax);
	      }

              break;
	    }
          }

        }
	else if(m>0)
        {
	  printf("c:%d e:%d m:%d unknow case\n",c, e, m);
	  goto next_elem;
	}
      }
      else { if(!msgFlag) { printf("\n ERROR: Found wrong element type. Only tets can be splitted!\n\n"); msgFlag=1; } }
      continue;
    next_elem:;
     seta(set_efail, "e",e);
    }
    if(param[0]=='e') goto ready;

    //completeSet( "all", "up" );
    //completeSet( "all", "do" );
    
    /* new midnodes */
    adjustDrawNodes(1);

    /* merge top nodes */
    sprintf(buffer,"n %s",set[set_ntop].name);
    pre_merge(buffer);

    makeSurfaces();
    getElemNormalen( e_enqire, node, anz->e );
    realloc_colNr();

    if(printFlag) printf(" updateDispLists\n");
    updateDispLists(); 

    completeSet( set[set_ntop].name, "do" );
    completeSet( set[set_nbot].name, "do" );
    separateMeshes( "all", "+splt");

  ready:;
    adjustDrawNodes(1);
    printf (" ready\n");
  }


  /* 1) split all lines */

  /* go over all lines */
  if(set[setNr].anz_l)
  {
    /* create a dummy set for the new lines and for the surfs to split */
    delSet("+splitline");
    delSet("+splitsurf");
    setSplit=pre_seta("+splitline","i",0);
    setSurf=pre_seta("+splitsurf","i",0);

    /* go over all lines and calculate the split-point */
    set[setNr].type=1;  // make sequence
    for (ll=0; ll<set[setNr].anz_l; ll++)
    {
      l=set[setNr].line[ll];
      if(printFlag)
	printf("check line:%s ip:%d\n", line[l].name, line[l].nip);

      /* store the line-dots in an array of points */
      lps=0.;

      pl[0][0]=line[l].ip[0];
      pl[0][1]=line[l].ip[1];
      pl[0][2]=line[l].ip[2];
      for (p=3; p<line[l].nip; p+=3)
      {
        pl[1][0]=line[l].ip[p]  ;
        pl[1][1]=line[l].ip[p+1];
        pl[1][2]=line[l].ip[p+2];
  
        /* The embedded triangles of the surfaces are used as a target */
        /* determine the position where the vector vproj between two line-dots meets the triangles */
        /* and determine if the position is inside the triangle */
        vproj[0]=pl[1][0]-pl[0][0];
        vproj[1]=pl[1][1]-pl[0][1];
        vproj[2]=pl[1][2]-pl[0][2];
        lp0p1=v_betrag( vproj );
        v_norm( vproj, eg );
        /*
        printf("pl: %lf %lf %lf\n",pl[0][0],pl[0][1],pl[0][2]);
        printf("v: %lf %lf %lf\n",vproj[0],vproj[1],vproj[2]);
        printf("pt1: %lf %lf %lf\n",pt1[0],pt1[1],pt1[2] );
        printf("pt2: %lf %lf %lf\n",pt2[0],pt2[1],pt2[2] );
        printf("pt3: %lf %lf %lf\n",pt3[0],pt3[1],pt3[2] );
        */

        /* go over all splitting-tri */
        for( n=0; n<sum_tri; n++)
        {
          pt1[0]=tri[n].p1[0];
          pt1[1]=tri[n].p1[1];
          pt1[2]=tri[n].p1[2];
          pt2[0]=tri[n].p2[0];
          pt2[1]=tri[n].p2[1];
          pt2[2]=tri[n].p2[2];
          pt3[0]=tri[n].p3[0];
          pt3[1]=tri[n].p3[1];
          pt3[2]=tri[n].p3[2];

	  // get the splitpoint psplt with the tri and check if it is inside the tri
	  
          v_result( pt1, pt2, p01 );
          v_norm( p01, eu );
          v_result( pt1, pt3, p01 );
          v_norm( p01, ev );
          v_prod( eu, ev, p01);
          v_norm( p01, en );
          /* bestimme den Abstand zwischen den Aufpunkten der Linie und Ebene  */
          v_result( pl[0], pt1, p01 );
          g = AsplitL( p01, eu, ev, eg, en );
          v_scal( &g, eg, p01 );
          v_add( pl[0], p01, &psplt[0][0] );
	  // projpl[0] and pl[1] onto en and generate new points there
          v_result( &psplt[0][0], pl[0], p01 );
	  g=v_sprod(p01,en);
	  v_scal(&g,en, p01);
	  v_add(&psplt[0][0],p01,pp1);
          v_result( &psplt[0][0], pl[1], p01 );
	  g=v_sprod(p01,en);
	  v_scal(&g,en, p01);
	  v_add(&psplt[0][0],p01,pp2);
	  
          dist=v_distA(  pt1, pt2, pt3, pp1, en, 1.01, &orient);
          dist2=v_distA(  pt1, pt2, pt3, pp2, en, 1.01, &orient);

          /* create a point only if the vector goes through the tri and if the sign of dist has changed (scip the first line-vector (p==3))*/
	  //printf("ip:%d nip:%d n:%d dist2:%e dist:%e dist2*dist:%e o:%d\n",p,line[l].nip,n, dist2, dist, dist2*dist, orient);
          if (( dist != OUTSIDE ) && ( dist2 != OUTSIDE ) && (dist2*dist <= 0.))
          {
            if(printFlag)
	      printf("found tri\n");
            v_result( pl[0], &psplt[0][0], vs );
            lp0p1=v_betrag( vs );
            lps+=lp0p1;

            /* split line only if its end-points are more than ltol away */
            mode=0;
            dx=psplt[0][0]-(point[line[l].p1].px);
            dy=psplt[0][1]-(point[line[l].p1].py);
            dz=psplt[0][2]-(point[line[l].p1].pz);
            if(sqrt(dx*dx+dy*dy+dz*dz)<ltol)
	    {
	      // remember the point for later surf splitting
	      ps_nr  = line[l].p1;
	      /* remember one new line and the split point for later use */
              if ( (splitbuf = (Splitbuf *)realloc((Splitbuf *)splitbuf, (sum_splitbuf+1)*sizeof(Splitbuf))) == NULL )
		printf("\n\n ERROR: malloc failure\n\n" );
	      splitbuf[sum_splitbuf].p=ps_nr;
	      splitbuf[sum_splitbuf].l=l;
	      sum_splitbuf++;
	      mode=1;
	      // printf("p1:%s d1:%e gtol:%e\n", point[line[l].p1].name, sqrt(dx*dx+dy*dy+dz*dz),ltol); 
	    }
            if(!mode)
	    {
              dx=psplt[0][0]-(point[line[l].p2].px);
              dy=psplt[0][1]-(point[line[l].p2].py);
              dz=psplt[0][2]-(point[line[l].p2].pz);
              if(sqrt(dx*dx+dy*dy+dz*dz)<ltol)
	      {
	        // remember the point for later surf splitting
	        ps_nr  = line[l].p2;
	        /* remember one new line and the split point for later use */
                if ( (splitbuf = (Splitbuf *)realloc((Splitbuf *)splitbuf, (sum_splitbuf+1)*sizeof(Splitbuf))) == NULL )
	          printf("\n\n ERROR: malloc failure\n\n" );
	        splitbuf[sum_splitbuf].p=ps_nr;
	        splitbuf[sum_splitbuf].l=l;
		sum_splitbuf++;
	        mode=1;
	        // printf("p2:%s d2:%e gtol:%e\n", point[line[l].p2].name, sqrt(dx*dx+dy*dy+dz*dz),ltol); 
	      }
	    }
            if(!mode)
	    {
              getNewName( name, "p" );
              if(printFlag)
		printf(" create split-point:%s %lf %lf %lf\n", name, psplt[0][0], psplt[0][1], psplt[0][2] );
              ps_nr  = pnt( name, psplt[0][0], psplt[0][1], psplt[0][2], 0 );
  
              /* delete the mesh on the related surfs */
	      
	      setMesh=pre_seta("-meshbuf","i",0);
              seta(setMesh, "l", l);
	      completeSet("-meshbuf", "up");
              sprintf(buffer,"me %s", set[setMesh].name);
	      k=0; for(i=0; i<set[setMesh].anz_l; i++) k+=line[set[setMesh].line[i]].nn;
              pre_del(buffer);
	      if(k) meshAgainFlag=1;
	      else meshAgainFlag=0;
              for(i=0; i<set[setMesh].anz_l; i++)
              {
                //printf("del mesh line %s\n",line[set[setMesh].line[i]].name);
                line[set[setMesh].line[i]].nn=0;
                line[set[setMesh].line[i]].ne=0;
              }
              for(i=0; i<set[setMesh].anz_s; i++)
              {
                //printf("del mesh surf %s\n",surf[set[setMesh].surf[i]].name);
                surf[set[setMesh].surf[i]].nn=0;
                surf[set[setMesh].surf[i]].ne=0;
		seta(setSurf, "s", set[setMesh].surf[i]);
              }
	      delSet("-meshbuf");
	      
              /* create two new lines l_nr and update the lcmbs and surfs */
	      //if(line[l].typ=='s') convertLine( l,line[l].div-1);
              if(splitLine(l, l_nr, ps_nr)==-1) { printf(" ERROR: Line splitting failed for:%s\n", line[l].name); continue; }
              seta(setSplit, "l", l_nr[0]);
              seta(setSplit, "l", l_nr[1]);
              seta(setNr, "l", l_nr[0]); 
              seta(setNr, "l", l_nr[1]); 
    
	      /* remember one new line and the split point for later use */
              if ( (splitbuf = (Splitbuf *)realloc((Splitbuf *)splitbuf, (sum_splitbuf+1)*sizeof(Splitbuf))) == NULL )
		printf("\n\n ERROR: malloc failure\n\n" );
	      splitbuf[sum_splitbuf].p=ps_nr;
	      splitbuf[sum_splitbuf].l=l_nr[0];
	      sum_splitbuf++;

              /* remember the basic line for later deletion */
              if ( (lbuffer = (int *)realloc((int *)lbuffer, (sum_l+1)*sizeof(int))) == NULL )
		printf("\n\n ERROR: malloc failure\n\n" );
              lbuffer[sum_l]=l;
              sum_l++;
	      
              goto nextLine;  // check no more tri's, take the next line
            }
	  }
        }
        lps+=lp0p1;
        pl[0][0]=pl[1][0];
        pl[0][1]=pl[1][1];
        pl[0][2]=pl[1][2];
      }
    nextLine:;
    }
    /* delete all splitted lines */
    delLine( sum_l, lbuffer );
    free(lbuffer);
    lbuffer=NULL;
    sum_l=0;
    /* add the new lines to the split set */
    s=getSetNr(setname);
    for(ll=0; ll<set[setSplit].anz_l; ll++) seta(s,"l",set[setSplit].line[ll]);
    delSet(set[setSplit].name);
    // should not be necessary
    completeSet("all", "do");


    /* 2) split surfaces */

    printf(" go over all surfs which should be splitted and create the split-points\n");
    
    if((sum_splitbuf>1)&&(set[setNr].anz_s>0))
    {
      /* sets for trackpoint adjustments of the new lines */
      delSet("-setp");
      delSet("-sets");
      setp=pre_seta("-setp","i",0);
      sets=pre_seta("-sets","i",0);
      // splitlines on one side, the other side will be generated in separateSurfs()
      delSet("+cut1");
      setSplit=pre_seta("+cut1","i",0);
  
      for (ss=0; ss<set[setNr].anz_s; ss++)
      {
        sum_linep=0;
        sum_surfp=0;
        s=set[setNr].surf[ss];
        
        /* buffer to relate surf and new lines */
        if ( (surfbuf = (Surfbuf *)realloc((Surfbuf *)surfbuf, (sum_surfbuf+1)*sizeof(Surfbuf))) == NULL )
          printf("\n\n ERROR: malloc failure\n\n" );
        surfbuf[sum_surfbuf].s=s;
        surfbuf[sum_surfbuf].nl=0;
        surfbuf[sum_surfbuf].l=NULL;
        surfbuf[sum_surfbuf].used=NULL;
        sum_surfbuf++;
        
        /* go over all internal tri */
        n=0;
        while((surf[s].npgn-n))
        {
          /* requires that only triangles are used for the interiour description of the surface */
          /* this is regarded by the adjustFeedBack() routine */
          n+=5; /* jump over the leading bytes */
          ps[0][0]=surf[s].pgn[n++];
          ps[0][1]=surf[s].pgn[n++];
          ps[0][2]=surf[s].pgn[n++];
          ps[1][0]=surf[s].pgn[n++];
          ps[1][1]=surf[s].pgn[n++];
          ps[1][2]=surf[s].pgn[n++];
          ps[2][0]=surf[s].pgn[n++];
          ps[2][1]=surf[s].pgn[n++];
          ps[2][2]=surf[s].pgn[n++];
    
          /* "cg" of tri-edges */
          cg_edge[0][0]=(ps[0][0]+ps[1][0])*.5;
          cg_edge[0][1]=(ps[0][1]+ps[1][1])*.5;
          cg_edge[0][2]=(ps[0][2]+ps[1][2])*.5;
    
          cg_edge[1][0]=(ps[1][0]+ps[2][0])*.5;
          cg_edge[1][1]=(ps[1][1]+ps[2][1])*.5;
          cg_edge[1][2]=(ps[1][2]+ps[2][2])*.5;
    
          cg_edge[2][0]=(ps[2][0]+ps[0][0])*.5;
          cg_edge[2][1]=(ps[2][1]+ps[0][1])*.5;
          cg_edge[2][2]=(ps[2][2]+ps[0][2])*.5;
    
          /* go over all edges and determine the split-point */
          sum_psplt=0;
          for(e=0; e<3; e++)
          {
            /* find the closest splitting-tri */
            /* distance between line-cg and tri-cg */
            if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
              printf("ERROR: realloc failed: Rsort\n\n" );
            for(i=0; i<sum_tri; i++)
            {
              dx=tri[i].cg[0]-cg_edge[e][0];
              dy=tri[i].cg[1]-cg_edge[e][1];
              dz=tri[i].cg[2]-cg_edge[e][2];
              rsort[i].r=dx*dx+dy*dy+dz*dz;
              rsort[i].i=i;
            }
            qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    
            //for (i=0; i<sum_tri; i++) printf("%d n:%d r:%lf\n", i, rsort[i].i, rsort[i].r); 
    
            /* check the closest tri for a split-point */
            for(i=0; i<sum_tri; i++)
            {
              /* The embedded triangles of the surfaces are used as a target */
              /* determine the position where the vector vproj between two line-dots meets the triangles */
              /* and determine if the position is inside the triangle */
              if(e==0)      { p2_nr=1; p1_nr=0; }
              else if(e==1) { p2_nr=2; p1_nr=1; }
              else          { p2_nr=0; p1_nr=2; }
              vproj[0]=ps[p2_nr][0]-ps[p1_nr][0];
              vproj[1]=ps[p2_nr][1]-ps[p1_nr][1];
              vproj[2]=ps[p2_nr][2]-ps[p1_nr][2];
              v_norm( vproj, eg );
    	  
              nn=rsort[i].i;
              v_result( tri[nn].p1, tri[nn].p2, p01 );
              v_norm( p01, eu );
              v_result( tri[nn].p1, tri[nn].p3, p01 );
              v_norm( p01, ev );
              v_prod( eu, ev, p01);
              v_norm( p01, en );
              /* bestimme den Abstand zwischen den Aufpunkten der Linie und Ebene  */
              v_result( ps[p1_nr], tri[nn].p1, p01 );
              g = AsplitL( p01, eu, ev, eg, en );
              v_scal( &g, eg, p01 );
              v_add( ps[p1_nr], p01, pc );  // pc==psplt
              // proj ps[p1_nr] and ps[p2_nr] onto en and generate new points there
              v_result( pc, ps[p1_nr], p01 );
              g=v_sprod(p01,en);
              v_scal(&g,en, p01);
              v_add(pc,p01,pp1);
              v_result( pc, ps[p2_nr], p01 );
              g=v_sprod(p01,en);
              v_scal(&g,en, p01);
              v_add(pc,p01,pp2);
    
              dist=v_distA(  tri[nn].p1, tri[nn].p2, tri[nn].p3, pp1, en, 1., &orient);
              dist2=v_distA(  tri[nn].p1, tri[nn].p2, tri[nn].p3, pp2, en, 1., &orient);
      
              /* create a point only if the vector goes through the tri and if the sign of dist has changed (scip the first line-vector (p==3))*/
    	    //if (( dist != OUTSIDE ) && ( dist2 != OUTSIDE ))  printf(" tri:%d dist2:%lf dist:%lf  %f\n",rsort[i].i, dist2, dist, dist2*dist);
              if (( dist != OUTSIDE ) && ( dist2 != OUTSIDE ) && (dist2*dist < 0.))
              {
                /* save split-point */
                psplt[sum_psplt][0]=pc[0];
                psplt[sum_psplt][1]=pc[1];
                psplt[sum_psplt][2]=pc[2];
    	    
                sum_psplt++;
                break;
              }
            }
          }

          /* create split-point at the edge of the surface-tri */
          if(sum_psplt==2)
          {
            for(j=0; j<2; j++)
            {
              //printf("check j%d\n",j);
              mode=1;
              // check if a point at this loc is already stored
              for(k=0; k<sum_surfp; k++)
              {
                v_result(&point[surfp[k]].px, &psplt[j][0], v12);
                dist=v_betrag(v12);
                //printf("check surfp:%d %s x:%f %f dist:%f\n",k,point[surfp[k]].name,point[surfp[k]].px,psplt[j][0],dist);
                if(dist<ltol) mode=0;
              }
           
              // check if a line split point has the same coordinates
              for(k=0; k<sum_splitbuf; k++)
              {
                v_result(&point[splitbuf[k].p].px, &psplt[j][0], v12);
                dist=v_betrag(v12);
                //printf("check linep:%d %s x:%f %f dist:%f\n",k,point[splitbuf[k].p].name,point[splitbuf[k].p].px,psplt[j][0],dist);
                if(dist<ltol) mode=0;
              }
          
              if(mode)
              {
                pc[0]=psplt[j][0];
                pc[1]=psplt[j][1];
                pc[2]=psplt[j][2];
                getNewName( name, "p" );
                if(printFlag)
                printf(" create surf-point:%s %lf %lf %lf\n", name, pc[0], pc[1], pc[2] );
                ps_nr  = pnt( name, pc[0], pc[1], pc[2], 0 );
        
                /* remember the point for later use */       
                if ( (surfp = (int *)realloc((int *)surfp, (sum_surfp+1)*sizeof(int))) == NULL )
                  printf("\n\n ERROR: malloc failure\n\n" );
                surfp[sum_surfp]=ps_nr;
                sum_surfp++;
              }
            }
          }
        }

	//printf(" generate lines connecting the split-points:%d using inner points:%d for splines\n", sum_splitbuf,sum_surfp);
        // remember all points and order according to the distance to splitbuf[].p
        // - search the splitbuf[].p which relate to the actual surface- (s) line (l) and store in linep
        // - start at the first of this linep and search the closest surfp
        // - search the closest surfp to the previous surfp
        // - until one of the remaining linep is closer.
        // - store them all in a new sequence and create a line.
        //if(sum_surfp)
        {
          // - search the splitbuf[].p which relate to the actual surface-line and store in linep[]
          for(i=0; i<sum_splitbuf; i++)
          {
            for (j=0; j<surf[s].nl; j++)
            {
              l=surf[s].l[j];
              if (surf[s].typ[j]=='l')
              {
                if(splitbuf[i].l==l)
                {
                  // is that point already stored?
                  for(k=0; k<sum_linep; k++) if(linep[k]==splitbuf[i].p) goto nextline;
                  //printf ("found %1c %s, store p%d %s\n", surf[s].o[j], line[l].name, sum_linep+1, point[splitbuf[i].p].name );
                  if ( (linep = (int *)realloc((int *)linep, (sum_linep+1)*sizeof(int))) == NULL )
                    printf("\n\n ERROR: malloc failure\n\n" );
                  linep[sum_linep]=splitbuf[i].p;
                  sum_linep++;
                }
              }
              else
              {
                for (k=0; k<lcmb[l].nl; k++ )
                {
                  ll=lcmb[l].l[k];
                  if(splitbuf[i].l==ll)
                  {
                    // is that point already stored?
                    for(jj=0; jj<sum_linep; jj++) if(linep[jj]==splitbuf[i].p) goto nextline;
                    //printf ("found lcmb:%s l:%s, store p%d %s\n", lcmb[l].name, line[ll].name, sum_linep+1, point[splitbuf[i].p].name);
                    if ( (linep = (int *)realloc((int *)linep, (sum_linep+1)*sizeof(int))) == NULL )
                      printf("\n\n ERROR: malloc failure\n\n" );
                    linep[sum_linep]=splitbuf[i].p;
                    sum_linep++;
                  }
                }
              }
            nextline:;
            }
          }
          
          // - start at the first of this linep and search the closest surfp
          //printf(" start at the first of this linep and search the closest surfp\n");
          if(sum_linep>1) for(k=0; k<sum_linep; k++)
          {
            //printf("linep k:%d\n",k);
            if(linep[k]==-1) continue;
            if(printFlag)
              printf("track starts at linep %s\n", point[linep[k]].name);
            n2=-1;
            dist1=MAX_FLOAT;
            // in case no surfp exist between two linep (coarse mesh) the next linep has to be taken
            for(i=0; i<sum_linep; i++)
            { 
              //printf("linep i:%d\n",i);
              if(i==k) continue;
              if(linep[i]==-1) continue;
              v_result(&point[linep[i]].px, &point[linep[k]].px, v12);
              dist=v_betrag(v12);
              //printf("check linep %s dist:%e\n", point[linep[i]].name, dist);
              if(dist<dist1) { dist1=dist; n2=i; }
            }
            if(n2==-1) { printf(" ERROR, no second splitpoint found\n"); continue; }
            p=-1;
            dist2=MAX_FLOAT;
            for(i=0; i<sum_surfp; i++)
            {
              if(surfp[i]==-1) continue;
              v_result(&point[linep[k]].px, &point[surfp[i]].px, v12);
              dist=v_betrag(v12);
              //printf("%d comp %d %d\n",i,linep[k],surfp[i]);
              //printf("comp %s %s dist:%f\n",point[linep[k]].name,point[surfp[i]].name,dist);
              if(dist<dist2) { dist2=dist; p=i; }
            }
            if(p==-1) printf(" WARNING: No surfpoints found\n");
            if(dist1<dist2)
            {
              if(printFlag)
                printf(" WARNING: No surfpoints between splitpoints found\n");
    
              // check if a line or lcmb with the same end-points exists already, then do not generate and store a new line
              p1_nr=linep[k];
              p2_nr=linep[n2];
              mode=0;
              for(i=0; i<surf[s].nl; i++)
              {
                l=surf[s].l[i];
                if(surf[s].typ[i]=='l')
                {
                  if((line[l].p1==p1_nr)&&(line[l].p2==p2_nr)) mode++;
                  if((line[l].p2==p1_nr)&&(line[l].p1==p2_nr)) mode++;
                  if(mode==1) { ltyp='l'; lnr=l; break; }
                  mode=0;
                }
                else
                {
                  if((lcmb[l].p1==p1_nr)&&(lcmb[l].p2==p2_nr)) mode++;
                  if((lcmb[l].p2==p1_nr)&&(lcmb[l].p1==p2_nr)) mode++;
                  if(mode==1) { ltyp='c'; lnr=l; break; }
                  mode=0;
                  for(m=0; m<lcmb[l].nl; m++)
                  {
                    ll=lcmb[l].l[m];
                    if((line[ll].p1==p1_nr)&&(line[ll].p2==p2_nr)) mode++;
                    if((line[ll].p2==p1_nr)&&(line[ll].p1==p2_nr)) mode++;
                    if(mode==1) { ltyp='l'; lnr=ll; break; }
                    mode=0;
                  }
                }
              }
              //printf(" mode:%d\n",mode);
  
              ibuf=2;
            
              if(mode==0)
              {
                getNewName( name, "l" );
                if(printFlag)
                  printf(" create line:%s %s %s %d\n", name, point[linep[k]].name, point[linep[n2]].name, ibuf );
                l=line_i( name, linep[k], linep[n2], 0, ibuf, 1, 0 );
    
                if ( (surfbuf[sum_surfbuf-1].l = (int *)realloc((int *)surfbuf[sum_surfbuf-1].l, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                  printf("\n\n ERROR: malloc failure\n\n" );
                if ( (surfbuf[sum_surfbuf-1].used = (int *)realloc((int *)surfbuf[sum_surfbuf-1].used, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                  printf("\n\n ERROR: malloc failure\n\n" );
                surfbuf[sum_surfbuf-1].l[surfbuf[sum_surfbuf-1].nl]=l;
                surfbuf[sum_surfbuf-1].used[surfbuf[sum_surfbuf-1].nl]=0;
                surfbuf[sum_surfbuf-1].nl++;
                linep[k]=-1;
                linep[n2]=-1;
              }
              else
              {
		if(ltyp=='c')
		{
		  printf(" ERROR: lcmb %s exists in the splitting location. The code can not deal with that.\n", lcmb[lnr].name);
		  continue;
		}
                if ( (surfbuf[sum_surfbuf-1].l = (int *)realloc((int *)surfbuf[sum_surfbuf-1].l, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                  printf("\n\n ERROR: malloc failure\n\n" );
                if ( (surfbuf[sum_surfbuf-1].used = (int *)realloc((int *)surfbuf[sum_surfbuf-1].used, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                  printf("\n\n ERROR: malloc failure\n\n" );
                surfbuf[sum_surfbuf-1].l[surfbuf[sum_surfbuf-1].nl]=lnr;
                surfbuf[sum_surfbuf-1].used[surfbuf[sum_surfbuf-1].nl]=1;
                surfbuf[sum_surfbuf-1].nl++;

		// search all connected surfs and add them to the surfs to be splitted
		for(kk=0; kk<set[setNr].anz_s; kk++)
		{
		  e=set[setNr].surf[kk];
		  for(j=0; j<surf[e].nl; j++)
		  {
		    l=surf[e].l[j];
		    if(surf[e].typ[j]=='l')
		    {
		      if(l==lnr)
		      {
			seta(setSurf, "s", e );
		      }
		    }
		    else
		    {
		      for (ii=0; ii<lcmb[l].nl; ii++ )
		      {
			ll=lcmb[l].l[ii];
			if(ll==lnr)
			{
		          seta(setSurf, "s", e );
			}
		      }
		    }
		  }
		}
              }
              continue;
            }
          
            // create a sequence and store the points
            getNewName( name, "se" );
            trkNr=pre_seta( name, "is", 0);
            seta( trkNr, "ps", linep[k] );
            linep[k]=-1;
            if(printFlag)
	      printf("add 1st pnt:%s\n",point[surfp[p]].name);
            seta( trkNr, "ps", surfp[p] );
            
            // - search the closest surfp to the previous surfp
            do
            {
              n1=n2=-1;
              distn=dist2=MAX_FLOAT;
              for(i=0; i<sum_surfp; i++)
              {
                if(i==p) continue;
                if(surfp[i]==-1) continue;
                v_result(&point[surfp[p]].px, &point[surfp[i]].px, v12);
                dist=v_betrag(v12);
                //printf("compare %s %s dist:%e\n",point[surfp[p]].name, point[surfp[i]].name, dist);
                if(dist<distn) { distn=dist; n1=i; }
              }
              //if(n1>-1) printf("closest point %s to %s dist:%f\n",point[surfp[n1]].name, point[surfp[p]].name, dist);
            
              // - until one of the remaining linep is closer.
              for(i=0; i<sum_linep; i++)
              { 
                if(linep[i]==-1) continue;
                v_result(&point[linep[i]].px, &point[surfp[p]].px, v12);
                dist=v_betrag(v12);
                //printf("check splitp %s %s dist:%e\n",point[surfp[p]].name, point[linep[i]].name, dist);
                if(dist<dist2) { dist2=dist; n2=i; }
              }
              if(n2==-1) break;
        
              if(dist2<distn)
              {
                if(printFlag)
                  printf(" found endpoint\n");
                seta( trkNr, "ps", linep[n2] );
                linep[n2]=-1;
      
                // check if a line or lcmb with the same end-points exists already, then do not generate and store a new line
                p1_nr=set[trkNr].pnt[0];
                p2_nr=set[trkNr].pnt[set[trkNr].anz_p-1];
                mode=0;
                for(i=0; i<surf[s].nl; i++)
                {
                  l=surf[s].l[i];
                  if(surf[s].typ[i]=='l')
                  {
                    if((line[l].p1==p1_nr)&&(line[l].p2==p2_nr)) mode++;
                    if((line[l].p2==p1_nr)&&(line[l].p1==p2_nr)) mode++;
                    if(mode==1) { ltyp='l'; lnr=l; break; }
                    mode=0;
                  }
                  else
                  {
                    if((lcmb[l].p1==p1_nr)&&(lcmb[l].p2==p2_nr)) mode++;
                    if((lcmb[l].p2==p1_nr)&&(lcmb[l].p1==p2_nr)) mode++;
                    if(mode==1) { ltyp='c'; lnr=l; break; }
                    mode=0;
                    for(m=0; m<lcmb[l].nl; m++)
                    {
                      ll=lcmb[l].l[m];
                      if((line[ll].p1==p1_nr)&&(line[ll].p2==p2_nr)) mode++;
                      if((line[ll].p2==p1_nr)&&(line[ll].p1==p2_nr)) mode++;
                      if(mode==1) { ltyp='l'; lnr=ll; break; }
                      mode=0;
                    }
                  }
                }
                //printf("2 mode:%d\n",mode);
                
                ibuf=((set[trkNr].anz_p-2)/4)*2+2;
                if(ibuf<2) ibuf=2;
                if(set[trkNr].anz_p<2) { printf("ERROR only %d points in trk:%s\n",set[trkNr].anz_p,set[trkNr].name); continue; }
              
                getNewName( name, "l" );
                if(printFlag)
                  printf(" create line:%s %s %s %s %d\n", name, point[set[trkNr].pnt[0]].name, point[set[trkNr].pnt[set[trkNr].anz_p-1]].name, set[trkNr].name, ibuf );
                ll=line_( name, point[set[trkNr].pnt[0]].name, point[set[trkNr].pnt[set[trkNr].anz_p-1]].name, set[trkNr].name, ibuf, 1 );
                if(mode!=0)
                {
                  if(printFlag)
                    printf(" compare the length of the lines. If equal, do not use the new line\n");
                  if(ltyp=='l') L1=calcLineLength(lnr); else L1=0.; //TBD
                  L2=calcLineLength(ll);
                  ql=abs(L2-L1)/L2;
                  if(printFlag)
                    printf(" length L1:%f L2:%f ql:%f\n",L1,L2,ql);
                  if(ql>0.001) mode=0;
                  else
                  {
                    delLine( 1, &ll);
                    if ( (surfbuf[sum_surfbuf-1].l = (int *)realloc((int *)surfbuf[sum_surfbuf-1].l, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                      printf("\n\n ERROR: malloc failure\n\n" );
                    if ( (surfbuf[sum_surfbuf-1].used = (int *)realloc((int *)surfbuf[sum_surfbuf-1].used, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                      printf("\n\n ERROR: malloc failure\n\n" );
                    surfbuf[sum_surfbuf-1].l[surfbuf[sum_surfbuf-1].nl]=lnr;
                    surfbuf[sum_surfbuf-1].used[surfbuf[sum_surfbuf-1].nl]=1;
                    surfbuf[sum_surfbuf-1].nl++;

                    // search all connected surfs and add them to the surfs to be splitted
                    for(kk=0; kk<set[setNr].anz_s; kk++)
                    {
                      e=set[setNr].surf[kk];
                      for(j=0; j<surf[e].nl; j++)
                      {
                        l=surf[e].l[j];
                        if(surf[e].typ[j]=='l')
                        {
                          if(l==lnr)
                          {
                            seta(setSurf, "s", e );
                          }
                        }
                        else
                        {
                          for (ii=0; ii<lcmb[l].nl; ii++ )
                          {
                            ll=lcmb[l].l[ii];
                            if(ll==lnr)
                            {
                              seta(setSurf, "s", e );
                            }
                          }
                        }
                      }
                    }
                  }
                } 
                if(mode==0)
                {
                  // change the internal points and delete the track to gain equally spaced points
        	  ii=line[ll].trk;
                  convertLine( ll,line[ll].div-1);
                  if ( (delpnts = (int *)malloc( (set[ii].anz_p) * sizeof(int))) == NULL )
                  { printf("\n\n ERROR  malloc\n\n") ; }
                  kk=0;
                  for(j=1;j<set[ii].anz_p-1; j++) delpnts[kk++]=set[ii].pnt[j];
                  delPnt( set[ii].anz_p-2, delpnts );
                  free(delpnts);
                  delSet(set[ii].name);
      
                  // adjust the trk-pnts to the surface interior and splitting entities by normal projections
                  // first normal to surf, then to split-tri and that in a loop until convergence.
                  set[setp].anz_p=0;
                  set[sets].anz_s=0;
                  set[sets].anz_sh=0;
                  set[sets].anz_nurs=0;
                  if(surf[s].sh>-1) seta(sets,"sh",surf[s].sh);
                  else seta(sets,"s",s);
                  for(j=1;j<set[line[ll].trk].anz_p-1; j++) seta(setp,"p",set[line[ll].trk].pnt[j]);
                  sprintf(buffer,"proj -setp %s nor\n",targetname);
                  sprintf(name,"proj -setp -sets nor\n");
                
                  // Break after convergence is reached
                  if ( (pntbuf = (double **)malloc( (set[setp].anz_p) * sizeof(double *))) == NULL )
                  { printf("\n\n ERROR  malloc\n\n") ; }
                  for(j=0;j<set[setp].anz_p; j++)
                  {
                    if ( (pntbuf[j] = (double *)malloc( (3) * sizeof(double))) == NULL )
                    { printf("\n\n ERROR  malloc\n\n") ; }
                    pntbuf[j][0]=point[set[setp].pnt[j]].px;
                    pntbuf[j][1]=point[set[setp].pnt[j]].py;
                    pntbuf[j][2]=point[set[setp].pnt[j]].pz;
                  }
                  for(ii=0; ii<50; ii++)
                  {
                    //printf(name);
                    pre_proj(name);
                    //printf(buffer);
                    pre_proj(buffer);
                    // check the biggest position change
                    dist2=0.;
                    for(j=0;j<set[setp].anz_p; j++)
                    {
                      v_result(&pntbuf[j][0],&point[set[setp].pnt[j]].px,p01);
                      dist=v_betrag(p01);
                      if( dist>dist2 ) dist2=dist;
                      pntbuf[j][0]=point[set[setp].pnt[j]].px;
                      pntbuf[j][1]=point[set[setp].pnt[j]].py;
                      pntbuf[j][2]=point[set[setp].pnt[j]].pz;
                    }
                    printf("line %s, projection loop %d, max pos change:%e, tol:%e\n",line[ll].name, ii,dist2,ltol);
                    if((dist2<ltol)||(dist2>distbuf))
		      break;
		    distbuf=dist2;
                  }
                  for(j=0;j<set[setp].anz_p; j++) free(pntbuf[j]);
                  free(pntbuf);
		  
                  if ( (surfbuf[sum_surfbuf-1].l = (int *)realloc((int *)surfbuf[sum_surfbuf-1].l, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                    printf("\n\n ERROR: malloc failure\n\n" );
                  if ( (surfbuf[sum_surfbuf-1].used = (int *)realloc((int *)surfbuf[sum_surfbuf-1].used, (surfbuf[sum_surfbuf-1].nl+1)*sizeof(int))) == NULL )
                    printf("\n\n ERROR: malloc failure\n\n" );
                  surfbuf[sum_surfbuf-1].l[surfbuf[sum_surfbuf-1].nl]=ll;
                  surfbuf[sum_surfbuf-1].used[surfbuf[sum_surfbuf-1].nl]=0;
                  surfbuf[sum_surfbuf-1].nl++;
                }
                break;
              }
              else
              {
                if(printFlag)
		  printf("add pnt:%s\n",point[surfp[n1]].name);
                seta( trkNr, "ps", surfp[n1] );
              }
          
              surfp[p]=-1;
              p=n1;
            }while(1);
          }
          if(printFlag)
            printf(" all lines generated for surf: %s\n\n",surf[s].name);
        }
        free(linep);
        free(surfp);
        linep=NULL;
        surfp=NULL;
      }

      /* lines are created, create new surfaces */
      if(printFlag)
        printf(" lines are created, create new surfaces\n");
      for(i=0; i<sum_surfbuf; i++)
      {
        if(!surfbuf[i].nl) continue;
        s_nr[0]=surfbuf[i].s;
	if(surf[s_nr[0]].c==NULL) { printf(" ERROR: Something wrong with surface:%s, splitting not possible\n", surf[s_nr[0]].name); continue; }
        seta(setSurf, "s", s_nr[0]);
        s_nr[1]=-1;
    
        // organize the surfbuf[i].l in a way that all lines between two splitpoints of the outer surface line-loop are grouped together
        // the lines i will be stored in linebuf[sum_linebuf].l[i]. These lines are connected by the inner surface lines.
        // sum_linebuf counts these linearrays
        sum_linebuf=0;
        for(k=0; k<surfbuf[i].nl; k++)
        {
          if(surfbuf[i].used[k]) continue;
  
          //check only the outer loop
          ibuf=0;
          for(j=0; j<surf[s_nr[0]].c[0]; j++)
          {
            //compare the 1st lpnt with the pnts of splitline, either both splitpnts must be found or just one (start a chain)
            l=surf[s_nr[0]].l[j];
            if(surf[s_nr[0]].typ[j]=='l')
            {
            //printf("x loop 0 check line %s with %s\n", line[l].name, line[surfbuf[i].l[k]].name);
              if(line[l].p1==line[surfbuf[i].l[k]].p1) ibuf++;
              if(line[l].p1==line[surfbuf[i].l[k]].p2) ibuf++;
              if(line[l].p2==line[surfbuf[i].l[k]].p1) ibuf++;
              if(line[l].p2==line[surfbuf[i].l[k]].p2) ibuf++;
            }
            else
            {
              for (ii=0; ii<lcmb[l].nl; ii++ )
              {
                ll=lcmb[l].l[ii];
                if(line[ll].p1==line[surfbuf[i].l[k]].p1) ibuf++;
                if(line[ll].p1==line[surfbuf[i].l[k]].p2) ibuf++;
                if(line[ll].p2==line[surfbuf[i].l[k]].p1) ibuf++;
                if(line[ll].p2==line[surfbuf[i].l[k]].p2) ibuf++;
              }
            }
          }
          // goes this line k straight through the surface?
          if(ibuf==4)
          {
            if ( (linebuf = (Linebuf *)realloc((Linebuf *)linebuf, (sum_linebuf+1)*sizeof(Linebuf))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            linebuf[sum_linebuf].nl=0;
            linebuf[sum_linebuf].l =NULL;
            if ( (linebuf[sum_linebuf].l = (int *)realloc((int *)linebuf[sum_linebuf].l, (linebuf[sum_linebuf].nl+1)*sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            linebuf[sum_linebuf].l[0]=surfbuf[i].l[k];
            linebuf[sum_linebuf].nl++;
            surfbuf[i].used[k]=1;
            sum_linebuf++;
          }
          else if(ibuf==2)  // starts a chain of lines until a subsequent line has a common point with the outer surface lines
          {
            if ( (linebuf = (Linebuf *)realloc((Linebuf *)linebuf, (sum_linebuf+1)*sizeof(Linebuf))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            linebuf[sum_linebuf].nl=0;
            linebuf[sum_linebuf].l =NULL;
            if ( (linebuf[sum_linebuf].l = (int *)realloc((int *)linebuf[sum_linebuf].l, (linebuf[sum_linebuf].nl+1)*sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            linebuf[sum_linebuf].l[0]=surfbuf[i].l[k];
            linebuf[sum_linebuf].nl++;
            surfbuf[i].used[k]=1;
  
            //printf(" starts a chain of lines until a subsequent line has a common point with the outer surface lines\n");
  
            // check all inner loops
            // (1) search the inner loop which connects to the current surfbuf[i].l[k]
            // (2) search the splitline which connects to the next inner loop or which ends at the outer loop 
  
            kk=k;
            if ( (usedLoops = (int *)calloc( (surf[s_nr[0]].nc+1), sizeof(int))) == NULL )
              printf("ERROR: realloc failed \n\n" ); 
        
            repeatWithInnerLoopSearch:;
        
            // eval (1)
            linoffs=0;
            for(jj=1; jj<surf[s_nr[0]].nc; jj++)
            {
              linoffs+=surf[s_nr[0]].c[jj-1];
              ibuf=0;
              if(usedLoops[jj]==1) continue;
              //printf("check loop %d \n",jj);
          
              for(j=linoffs; j<surf[s_nr[0]].c[jj]+linoffs; j++)
              {
                //compare the 1st lpnt with the pnts of splitline, either both splitpnts must be found or just one (start a chain)
                l=surf[s_nr[0]].l[j];
                if(surf[s_nr[0]].typ[j]=='l')
                {
                  //printf("loop %d check line %s with %s\n",jj, line[l].name, line[surfbuf[i].l[kk]].name);
                  if(line[l].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                  if(line[l].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                  if(line[l].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                  if(line[l].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                }
                else
                {
                  for (ii=0; ii<lcmb[l].nl; ii++ )
                  {
                    ll=lcmb[l].l[ii];
                    if(line[ll].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                    if(line[ll].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                    if(line[ll].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                    if(line[ll].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                  }
                }
              }
              // one point in common
              if(ibuf==2)
              {
                // eval (2), go over all surfbuf[i].l[kk] and store all lines which are connected with both ends to that loop
                //printf(" found inner surface line loop jj:%d, keep it and search next line surfbuf[i].l[kk]\n", jj);
                usedLoops[jj]=1;
                for(kk=0; kk<surfbuf[i].nl; kk++)
                {
                  if(surfbuf[i].used[kk]) continue;
                  ibuf=0;
                  for(j=linoffs; j<surf[s_nr[0]].c[jj]+linoffs; j++)
                  {
                    //compare the 1st lpnt with the pnts of splitline, either both splitpnts must be found or just one (start a chain)
                    l=surf[s_nr[0]].l[j];
                    if(surf[s_nr[0]].typ[j]=='l')
                    {
                      if(line[l].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                      if(line[l].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                      if(line[l].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                      if(line[l].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                    }
                    else
                    {
                      for (ii=0; ii<lcmb[l].nl; ii++ )
                      {
                        ll=lcmb[l].l[ii];
                        if(line[ll].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                        if(line[ll].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                        if(line[ll].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                        if(line[ll].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                      }
                    }
                  }
                  // two points in common, internal line
                  if(ibuf==4)
                  {
                    //printf("found inner line %s in loop:%d\n", line[surfbuf[i].l[kk]].name, jj);
                    if ( (linebuf[sum_linebuf].l = (int *)realloc((int *)linebuf[sum_linebuf].l, (linebuf[sum_linebuf].nl+1)*sizeof(int))) == NULL )
                      printf("\n\n ERROR: malloc failure\n\n" );
                    linebuf[sum_linebuf].l[linebuf[sum_linebuf].nl]=surfbuf[i].l[kk];
                    linebuf[sum_linebuf].nl++;
                    surfbuf[i].used[kk]=1;
                  }
                }
    
                // all inner lines are found, search border line
                kkbuf=-1;
                for(kk=0; kk<surfbuf[i].nl; kk++)
                {
                  if(surfbuf[i].used[kk]) continue;
                  ibuf=0;
                  for(j=linoffs; j<surf[s_nr[0]].c[jj]+linoffs; j++)
                  {
                    //compare the 1st lpnt with the pnts of splitline, either both splitpnts must be found or just one (start a chain)
                    l=surf[s_nr[0]].l[j];
                    if(surf[s_nr[0]].typ[j]=='l')
                    {
                      if(line[l].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                      if(line[l].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                      if(line[l].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                      if(line[l].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                    }
                    else
                    {
                      for (ii=0; ii<lcmb[l].nl; ii++ )
                      {
                        ll=lcmb[l].l[ii];
                        if(line[ll].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                        if(line[ll].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                        if(line[ll].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                        if(line[ll].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                      }
                    }
                  }
                  // one point in common, internal line
                  if(ibuf==2)
                  {
                    //printf("found outer line %s in loop:%d\n", line[surfbuf[i].l[kk]].name, jj);
                    if ( (linebuf[sum_linebuf].l = (int *)realloc((int *)linebuf[sum_linebuf].l, (linebuf[sum_linebuf].nl+1)*sizeof(int))) == NULL )
                      printf("\n\n ERROR: malloc failure\n\n" );
                    linebuf[sum_linebuf].l[linebuf[sum_linebuf].nl]=surfbuf[i].l[kk];
                    linebuf[sum_linebuf].nl++;
                    surfbuf[i].used[kk]=1;
                    kkbuf=kk;
                  }    
                }
                if(kkbuf==-1)
                {
                  printf("ERROR: found no connecting line \n");
                  goto nextsurfbuf;
                }
          
                // the current surfbuf[i].l[kk] is now either connected to the next inner loop or to the outer loop which
                // would stop the search
                kk=kkbuf;
          
                // check if that surfbuf[i].l[kk] connects to the outer loop
                ibuf=0;
                for(j=0; j<surf[s_nr[0]].c[0]; j++)
                {
                  //compare the 1st lpnt with the pnts of splitline, either both splitpnts must be found or just one (start a chain)
                  l=surf[s_nr[0]].l[j];
                  if(surf[s_nr[0]].typ[j]=='l')
                  {
                    if(line[l].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                    if(line[l].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                    if(line[l].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                    if(line[l].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                  }
                  else
                  {
                    for (ii=0; ii<lcmb[l].nl; ii++ )
                    {
                      ll=lcmb[l].l[ii];
                      if(line[ll].p1==line[surfbuf[i].l[kk]].p1) ibuf++;
                      if(line[ll].p1==line[surfbuf[i].l[kk]].p2) ibuf++;
                      if(line[ll].p2==line[surfbuf[i].l[kk]].p1) ibuf++;
                      if(line[ll].p2==line[surfbuf[i].l[kk]].p2) ibuf++;
                    }
                  }
                }
                // one point in common, line is connected to outer loop, search stops

                if(ibuf==2) goto stopLineSearch;
                else goto repeatWithInnerLoopSearch;

              }
        
            }
          stopLineSearch:;
            free(usedLoops);
            sum_linebuf++;
          }
        }
        /*
        for(k=0; k<sum_linebuf; k++)
        {
          printf("line chain:%d\n",k+1);
          for(j=0; j<linebuf[k].nl; j++) printf("connected splitline[%d]: %s\n",j, line[linebuf[k].l[j]].name);
        }
        */
        // split the surface
        // in case sum_linebuf>1: The surface was cutted several times. Then the surf to be cutted must be updated sequentially.
        for(k=0; k<sum_linebuf; k++)
        {
          if(s_nr[1]>-1)  // update surf earliest at second loop k
          {
            // search the right surface for the next line k
            ss=0;
            for(s=0; s<2; s++)
            {
              ibuf=0;
              //printf("s:%d s_nr[s]:%d surf[s_nr[s]].nl:%d\n",s,s_nr[s],surf[s_nr[s]].nl);
              for(j=0; j<surf[s_nr[s]].c[0]; j++)
              {
                //compare the linepnts with the pnts of splitline, both splitpnts must be found
                l=surf[s_nr[s]].l[j];
                if(surf[s_nr[s]].typ[j]=='l')
                {
                  if(line[l].p1==line[linebuf[k].l[0]].p1) ibuf++;
                  if(line[l].p1==line[linebuf[k].l[0]].p2) ibuf++;
                  if(line[l].p2==line[linebuf[k].l[0]].p1) ibuf++;
                  if(line[l].p2==line[linebuf[k].l[0]].p2) ibuf++;
                  if(linebuf[k].nl>1)
                  {
                    jj=linebuf[k].nl-1;
                    if(line[l].p1==line[linebuf[k].l[jj]].p1) ibuf++;
                    if(line[l].p1==line[linebuf[k].l[jj]].p2) ibuf++;
                    if(line[l].p2==line[linebuf[k].l[jj]].p1) ibuf++;
                    if(line[l].p2==line[linebuf[k].l[jj]].p2) ibuf++;
                  }
                }
                else
                {
                  for (ii=0; ii<lcmb[l].nl; ii++ )
                  {
                    ll=lcmb[l].l[ii];
                    if(line[ll].p1==line[linebuf[k].l[0]].p1) ibuf++;
                    if(line[ll].p1==line[linebuf[k].l[0]].p2) ibuf++;
                    if(line[ll].p2==line[linebuf[k].l[0]].p1) ibuf++;
                    if(line[ll].p2==line[linebuf[k].l[0]].p2) ibuf++;
                    if(linebuf[k].nl>1)
                    {
                      jj=linebuf[k].nl-1;
                      if(line[ll].p1==line[linebuf[k].l[jj]].p1) ibuf++;
                      if(line[ll].p1==line[linebuf[k].l[jj]].p2) ibuf++;
                      if(line[ll].p2==line[linebuf[k].l[jj]].p1) ibuf++;
                      if(line[ll].p2==line[linebuf[k].l[jj]].p2) ibuf++;
                    }
                  }
                }
              }
              if(ibuf>3) { s_nr[0]=s_nr[s]; ss=1; }
            }
            if(!ss) { printf(" ERROR: No surf found for line:%s\n",line[linebuf[k].l[0]].name); continue; }
          }
            
          // split the surface
          s_nr[1]=splitSurf( linebuf[k].nl, &linebuf[k].l[0], s_nr[0], 0 );
          if(s_nr[1]<0)
          {
            printf(" ERROR:%d: could not split the selected surface\n",s_nr[1]);
            printf(" -1: malloc failed                        \n");
            printf(" -2: no line or surf selected                    \n");
            printf(" -4: the lines form only one loop, so no 2 surfs can be generated\n");
            printf(" -5: other error\n");
            goto nextsurfbuf;
          }
          else seta(setSurf, "s", s_nr[1]);
        }
      nextsurfbuf:;
      }
  
      /* separate the splitted surfs */
      
      // compile all splitting lines
      for(i=0; i<sum_surfbuf; i++)
      {
        for(j=0; j<surfbuf[i].nl; j++)
        {
           seta(setSplit, "l", surfbuf[i].l[j]);
        }
      }
      // separate the splitted surfs
      if(( compare( param, "nos", 3)!=3)&&(set[setSurf].anz_s>0))
        separateSurfs(setSplit, setSurf);

      delSet("-setp");
      delSet("-sets");
      
      // should not be necessary
      //completeSet("all", "do");
      
    } // end split surfs

    
    /* 3) split all bodies (currently just one body per split is allowed, otherwhise only the surfaces of the two sides are separated)  */ 

    // generate new surfaces in +cut1 and +cut2
    if(( compare( param, "nos", 3)!=3)&&(set[setSurf].anz_s>0))
    {
      // place the bodies to be splitted in +bod1
      delSet("+bod1");
      setB1=pre_seta("+bod1","i",0);
      for(i=0; i<set[setNr].anz_b; i++)
      {
        b=set[setNr].body[i];
        seta(setB1,"b",b);
      }
	
      // try to generate surfaces using the splitting lines
      
      // split +cut1 into closed line-loops
      // generate surfaces and add to +surf1
      // generate body using +surf1, store in +bod1

      if((set[trgtNr].anz_sh==1)&&(set[trgtNr].anz_s<=1))
      {
        delSet("-copy");
        ss=pre_seta("-copy","i",0);
        sprintf(buffer,"%s %s tra 0. 0. 0.", set[trgtNr].name, set[ss].name );
        pre_copy(buffer);
        en1=set[ss].shp[0];
        delSet("-copy");
        ss=pre_seta("-copy","i",0);
        sprintf(buffer,"%s %s tra 0. 0. 0.", set[trgtNr].name, set[ss].name );
	pre_copy(buffer);
        en2=set[ss].shp[0];
        delSet("-copy");
      }
      else en1=en2=-1;
      if ( (lbuffer = (int *)realloc((int *)lbuffer, (set[setSplit].anz_l+1) * sizeof(int))) == NULL )
        printf(" ERROR: realloc failed\n\n");
      for(i=0; i<set[setSplit].anz_l; i++) lbuffer[i]=set[setSplit].line[i];
      nsurfs=surfsFromCut( set[setSplit].anz_l, lbuffer, en1, &innerSurf );
      free(lbuffer);
      lbuffer=NULL;
      n=getSetNr("+set1"); 
      if(en1>-1) seta(n,"sh",en1);

      // set the surface element attributes
      // check the orientation:
      //   search a common line between +cut1 (setSplit) and this surf
      //   search a common line between +cut1 (setSplit) and a surf from +set1 (n): s1
      //   compare the orentation of this line in s and s1
      for(i=0; i<nsurfs; i++)
      {
        s=innerSurf[i];
	flipFlag=-1;
	for(j=0; j<surf[s].nl; j++)
	{
	  l1=surf[s].l[j];
          l=getIndex(&set[setSplit].line,set[setSplit].anz_l,l1);
	  if(l>-1) l=set[setSplit].line[l];
	  if(l==l1) break;
	}
	if(j==surf[s].nl) { printf(" ERROR in split, common line not found\n"); return; }
	i1=j;
	//printf(" common:%s i1:%d\n",line[l1].name, i1);
	
	for(k=0; k<set[n].anz_s; k++)
	{
	  s1=set[n].surf[k];
	  for(j=0; j<surf[s1].nl; j++)
	  {
	    l2=surf[s1].l[j];
          if(l2==l1) break;
	  }
        if(l2==l1) break;
	}
	if(k==set[n].anz_s) { printf(" ERROR in split, common surf not found\n"); return; }
	i2=j;
	//printf(" surf:%s i2:%d\n",surf[s1].name, i2);
	if((n>-1)&&((surf[set[n].surf[k]].etyp==8)||(surf[set[n].surf[k]].etyp==10))) e=8; else e=7;

        if(l2==l1)
	{
	  //printf("  o:%c %c\n",surf[s].o[i1],surf[s1].o[i2]);
	  if(surf[s].o[i1]==surf[s1].o[i2]) flipFlag=1;
	  if(surf[s].ori!=surf[s1].ori) flipFlag*=-1;
	  if(flipFlag==1) { if(surf[s].ori=='+') surf[s].ori='-'; else surf[s].ori='+'; }
	}
        surf[s].etyp=e;
        surf[s].eattr=-1;
        surf[s].elock=0;
	repSurf(s,1);
      }
      for(i=0; i<nsurfs; i++)
      {
        s=innerSurf[i];
        if(n>-1) seta(n,"s",s);
	  seta(setSplit,"s",s);
      }

      // care about +set2
	
      // split +cut2 into closed line-loops
      // generate surfaces and add to +surf2
      // generate body using +surf2, store in +bod2

      nn=getSetNr("+cut2"); 
      if ( (lbuffer = (int *)realloc((int *)lbuffer, (set[nn].anz_l+1) * sizeof(int))) == NULL )
      printf(" ERROR: realloc failed\n\n");
      for(i=0; i<set[nn].anz_l; i++) lbuffer[i]=set[nn].line[i];
      nsurfs=surfsFromCut( set[nn].anz_l, lbuffer, en2, &innerSurf );
      free(lbuffer);
      lbuffer=NULL;
      n=getSetNr("+set2"); 
      if(en2>-1) seta(n,"sh",en2);

      // set the surface element attributes
      // check the orientation
      // search a common line between +cut2 (nn) and this surf
      // search a common line between +cut2 (nn) and a surf from +set2 (n): s1
      // compare the orentation of this line in s and s1
      for(i=0; i<nsurfs; i++)
      {
        s=innerSurf[i];
        if(n>-1) seta(n,"s",s);
	flipFlag=-1;
	for(j=0; j<surf[s].nl; j++)
	{
	  l1=surf[s].l[j];
          l=getIndex(&set[nn].line,set[nn].anz_l,l1);
	  if(l>-1) l=set[nn].line[l];
	  if(l==l1) break;
	}
	if(j==surf[s].nl) { printf(" ERROR in split, common line not found\n"); return; }
	i1=j;
	//printf(" common:%s i1:%d\n",line[l1].name, i1);
	
	for(k=0; k<set[n].anz_s; k++)
	{
	  s1=set[n].surf[k];
	  for(j=0; j<surf[s1].nl; j++)
	  {
	    l2=surf[s1].l[j];
            if(l2==l1) break;
	  }
          if(l2==l1) break;
	}
	if(k==set[n].anz_s) { printf(" ERROR in split, common surf not found\n"); return; }
	i2=j;
	//printf(" surf:%s i2:%d\n",surf[s1].name, i2);
	if((n>-1)&&((surf[set[n].surf[k]].etyp==8)||(surf[set[n].surf[k]].etyp==10))) e=8; else e=7;

        if(l2==l1)
        {
	  //printf("  o:%c %c\n",surf[s].o[i1],surf[s1].o[i2]);
	  if(surf[s].o[i1]==surf[s1].o[i2]) flipFlag=1;
	  if(surf[s].ori!=surf[s1].ori) flipFlag*=-1;
	  if(flipFlag==1) { if(surf[s].ori=='+') surf[s].ori='-'; else surf[s].ori='+'; }
        }
        surf[s].etyp=e;
        surf[s].eattr=-1;
        surf[s].elock=0;
        repSurf(s,1);
      }

      // generate new bodies and delete the splitted ones

      // go over all bodies in +setB1 b
      // place all common surfs between b and +set1 in +setB2
      // add all surfs from innerSurf[i] (needs two arrays!) 
      if(set[setB1].anz_b==1)
      {
        sprintf(name,"! +set1\n");
        b=pre_body(name);
        n=getSetNr("+set1"); 
        seta(n,"b",b);
        sprintf(name,"! +set2\n");
        b=pre_body(name);
        n=getSetNr("+set2"); 
        seta(n,"b",b);
      }
      delBody( set[setB1].anz_b, set[setB1].body);
      completeSet("+set1", "do");
      completeSet("+set2", "do");
      delSet("+bod1");
    }
      
    // mesh again
    if(meshAgainFlag)
    {
      if(set[setSurf].anz_s)
      {
        for(i=0; i<set[setSurf].anz_s; i++)
        {
          printf("mesh %s\n",surf[set[setSurf].surf[i]].name);
        }
        completeSet(set[setSurf].name, "do");
        pre_mesh( set[setSurf].name );
        sprintf(buffer,"n %s",set[setSurf].name);
        pre_merge(buffer);
        printf(" %d edges in the model\n", anz->g);
      }
    }
    
    delSet("+splitline");
    delSet("+splitsurf");
  
  } // end split lines, surfs, bodies
  
  free(splitbuf);
  free(lbuffer);
  free(tri);
  free(rsort);
    
  for(i=0; i<sum_surfbuf; i++) free(surfbuf[i].l);
  free(surfbuf);
  delSet("-splitdep");
  delSet("-splitind");
  return;
}



/* liefert MAX_INTEGER wenn der node ausserhalb liegt, */
/* ansonsten den kuerzesten Abstand zur linie.     */
double check_line( double *pp, double *p0, double *p1, int tolflag )
{
  double v01[3],v10[3],p01[3],p10[3],ps[3],e01[3],p0ps[3],pspp[3];
  double sp1,sp2,l;
  #define STRETCHFAC 1.02

  /* stretch the line p0p1 a bit */
  v_result( p0, p1, v01  );
  l=v_norm(v01,e01);
  sp1=STRETCHFAC*l;
  v_scal(&sp1,e01,p0ps);
  v_add(p0,p0ps,p1);
  sp1=(1-STRETCHFAC)*l;
  v_scal(&sp1,e01,p0ps);
  v_add(p0,p0ps,p0);
  
  v_result( p0, p1, v01  );
  v_result( p1, p0, v10  );
  v_result( p0, pp, p01  );
  sp1=v_sprod(p01,v01);
  v_result( p1, pp, p10  );
  sp2=v_sprod(p10,v10);
  if((sp1>=0.)&&(sp2>=0.))
  {
    v_norm(v01,e01);
    sp1=v_sprod(p01,e01);
    v_scal(&sp1,e01,p0ps);
    v_add(p0,p0ps,ps);
    v_result(ps,pp,pspp);
    return(v_betrag(pspp));
  }
  else return(MAX_INTEGER);
}



/* liefert MAX_INTEGER wenn der node ausserhalb liegt, */
/* ansonsten den kuerzesten Abstand zur ebene.     */
double check_tri3( double *pp, double *pt1, double *pt2, double *pt3, int tolflag )
{
   int i;
   int inull, iplus, iminus;
   double  vN0N1[3], vN1N2[3], vN2N0[3], vN0N[3], vN1N[3],  vN2N[3],
          vprod0[3], vprod1[3], vprod2[3], vsprod[3], vnorm[3];
   double en[3], ev[3], eu[3], vNN0[3], g, a=0.;

   /* der node liegt im Dreieck wenn: */
   /* - berechne die Vektorprodukte zwischen den Kantenvektoren und dem Vektor von den    */
   /*   Kantenpunkten zu dem node (v_prod= vP1P2 x vP1Pn)                                 */
   /* - berechne die Scalarprtodukte v_sprod zwischen den v_prod und dem Normalenvektor   */
   /*   des Dreiecks. Wenn alle v_sprod positiv sind, dann liegt der node innerhalb und   */
   /*   davor, wenn alle negativ sind dann dahinter. Wenn uneinheitlich, dann ausserhalb. */

   v_result( pt1, pt2, vN0N1);
   v_result( pt2, pt3, vN1N2);
   v_result( pt3, pt1, vN2N0);
   v_result( pt1, pp, vN0N);
   v_result( pt2, pp, vN1N);
   v_result( pt3, pp, vN2N);

   /* berechne den Normalenvektor auf der flaeche */
   v_prod( vN0N1, vN1N2, vnorm );
   v_norm( vnorm, en );
   /* berechne die einheitsvektoren der ebenengleichung des tri3 */
   v_norm( vN0N1, eu );
   v_norm( vN1N2, ev );
   /* bestimme den Abstand zwischen den Aufpunkten der Normalen und in n verschobenen Ebene  */
   v_result( pp, pt1, vNN0 );

   /* berechne die Konstante g (Abstand)  pn_neu=pn-en*g  */
   g = AsplitL( vNN0, eu, ev, en, en );
   v_prod( vN0N1, vN0N, vprod0 );
   v_prod( vN1N2, vN1N, vprod1 );
   v_prod( vN2N0, vN2N, vprod2 );
   vsprod[0]= v_sprod( vnorm, vprod0);
   vsprod[1]= v_sprod( vnorm, vprod1);
   vsprod[2]= v_sprod( vnorm, vprod2);

   /*   kontrolle ob ein vsprod=0 ist. Dann liegt naemlich                */
   /*   der zu kontrollierende node genau ueber einer der umrandungen     */
   /*   der kontroll-flaeche (vprod ist dann senkrecht auf der normalen). */
   /*   Dann werden vsprod mit den gleichen vorzeichen zusammen-          */
   /*   gezaelt, dabei gelten die nuller als joker                        */

   /* setze sehr kleine zahlen zu NULL */
   if(tolflag)
   {
     for (i=0; i<3; i++) a+=abs(vsprod[i]);
     a*=1e-10;
     for (i=0; i<3; i++) if (abs(vsprod[i])<a) vsprod[i]=0.;
   }

   inull=0;
   for (i=0; i<3; i++) if (vsprod[i] == 0.) inull++;

   iplus=inull;
   for (i=0; i<3; i++) if (vsprod[i] > 0.) iplus++;

   iminus=inull;
   for (i=0; i<3; i++) if (vsprod[i] < 0.) iminus++;

   //printf (" vsprod: %le %le %le inull:%d iplus:%d iminus:%d offs:%lf a:%e\n", vsprod[0],vsprod[1],vsprod[2], inull, iplus, iminus, g, a );

   /*           * wenn alle vectorprodukte das gleiche vorzeichen haben    *   */
   /*           * dann ist der node innerhalb der flaeche                  *   */

   if (iplus == 3) return(g);
   else if (iminus == 3) return(g);
   else return(MAX_INTEGER);
}


/*
  if(compare(action,"PROJ",4)==4) functionFlag=1;
  else functionFlag=0; ("DIST")
*/
void pre_proj( char *string)
{
#define INTPOLMODE  0 /* linear */
#define PROJ_BIAS 1.1
#define PROJ_LOOPS 9 // must be <10 since the value is used as an exponent to 10 to calc an int value

  int  length, i,j=0,n,e,p, Stmp, setNr, trgtNr, mode, csys[3], surfFlag=0, normFlag=0, functionFlag, counter=0, orient;
  char setname[MAX_LINE_LENGTH], targetname[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH];
  char axis;
  double pr[3], pc[3], nr[3], pa1[3], pa2[3], pa3[3], ps1[3], ps2[3], vproj[3],v1[3],v2[3],bv1,bv2, offset=0., distn, dist=1.e33, mindist=0, mindistini=0.;
  double p01[3],eu[3],ev[3],en[3],eg[3],g,psplt[3];
  int  minj=0, anz_n=0;
  double pt1[3], pt2[3], pt3[3], rp, triScale=0.;
  double *nx=NULL;
  double *nrad=NULL;
  int  *nmissed=NULL;

  char pkt1[MAX_LINE_LENGTH], pkt2[MAX_LINE_LENGTH];
  int   pnr1=0, pnr2=0, setdep, setind;
  double r, fi, x, y, z, l, h, q, l_offs, h_offs, q_offs;
  double p1[3], p2[3], p1p2[3];
  double el[3], eh[3], eq[3];   /* xyz koordinaten der lhq vektoren */
  double ex[3], ey[3], ez[3];   /* lhq koordinaten der xyz vektoren */
  double ph[3], p1ph[3], pp[3], ppt[3], e1[3], lhq[3], angle;
  double pval, qval, divpq, scal_e[2];

  double v_qdis[2][3], da, dax, day, daz;

  double dist_max=-MAX_FLOAT,dist_min=MAX_FLOAT, sum_dist=0.;
  double dist_maxx=-MAX_FLOAT,dist_minx=MAX_FLOAT, distx;
  double dist_maxy=-MAX_FLOAT,dist_miny=MAX_FLOAT, disty;
  double dist_maxz=-MAX_FLOAT,dist_minz=MAX_FLOAT, distz;
  int ndist=0;

  Rsort *rsort=NULL;
  Tri *tri=NULL;
  int sum_tri=0;

  Points    *pproj=NULL;
  int sum_pproj;
  char *record=NULL;
 
  int     n_closest_tri=0, curr_n_closest_tri=0, min_n_closest_tri;
  double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL, *near_tri=NULL;

  /* get the function name */
  i=0; while(string[i]==' ') i++;
  i+= sword( string, action);
  record=string+i;

  /* check if the function 'dist' or 'proj' was selected */
  for(i=0; i<strlen(action); i++) action[i]=toupper(action[i]);
  if(compare(action,"PROJ",4)==4) functionFlag=1;
  else functionFlag=0;

  /* get basic parameters */
  action[0]=0;
  length = sscanf( record,"%s %s %s", setname, targetname, action );
  //printf("setname:%s %s %s\n",setname,targetname,action);
  operateAlias( setname, "se" );
  setNr=getSetNr( setname );
  if (setNr<0)
  {
    errMsg(" ERROR: Set (%s) is undefined\n", setname );
    return;
  }

  delSet(specialset->dep );
  delSet(specialset->ind );


  /* first assume the target is a single shape */
  operateAlias( targetname, "sh" );
  trgtNr=getShapeNr( targetname );
  if (trgtNr>-1)
  {
    if( (setind=pre_seta( specialset->ind, "sh", targetname )) <0 )
    {
      printf("ERROR: shape:%s could not be used\n", shape[trgtNr].name);
      return;
    }
    trgtNr=-1;
  }
  else
  {
    operateAlias( targetname, "se" );
    trgtNr=getSetNr( targetname );
    if (trgtNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", targetname );
      return;
    }

    /* cycle through all entities of targetNr and add them to the special set  */
    /* cyrcle through all bodies and add  */
    if( (setind=pre_seta( specialset->ind, "i", 0 )) <0 ) return;
    for (i=0; i<set[trgtNr].anz_s; i++)
    {
      seta( setind, "s", set[trgtNr].surf[i] );
    }
    for (i=0; i<set[trgtNr].anz_l; i++)
    {
      seta( setind, "l", set[trgtNr].line[i] );
    }
    for (i=0; i<set[trgtNr].anz_f; i++)
    {
      seta( setind, "f", set[trgtNr].face[i] );
    }
    for (i=0; i<set[trgtNr].anz_n; i++)
    {
      seta( setind, "n", set[trgtNr].node[i] );
    }
    /* add only shell elements */
    for (i=0; i<set[trgtNr].anz_e; i++)
    {
      if((e_enqire[set[trgtNr].elem[i]].type >6)&&(e_enqire[set[trgtNr].elem[i]].type <11)) seta( setind, "e", set[trgtNr].elem[i] );
    }
    /* second cycle through all entities and add lower ones  to the special set  */
    completeSet( specialset->ind, "do") ;
    completeSet( specialset->ind, "do") ;

    /* do not regard surface referenced shapes */
    set[setind].anz_nurs=0;
    set[setind].anz_sh=0;

    /* add user chosen shapes and nurbs */
    for (i=0; i<set[trgtNr].anz_nurs; i++)
    {
      seta( setind, "S", set[trgtNr].nurs[i] );
    }
    for (i=0; i<set[trgtNr].anz_sh; i++)
    {
      seta( setind, "sh", set[trgtNr].shp[i] );
    }
  }

  /* stop here if just a measurement between points or nodes should be performed */
  if( (!functionFlag)&&( (set[setNr].anz_p==1)||(set[setNr].anz_n==1)) && (action[0]==0) )
  {
    // if just one trgt point, do additional angle and radial measurements
    if((set[trgtNr].anz_p==1)||(set[trgtNr].anz_n==1))
    {
      if(set[trgtNr].anz_p==1)
      {
        if(set[setNr].anz_p==1)
        {
          v_qdis[0][0]=point[set[setNr].pnt[0]].px*scale->w+scale->x;
          v_qdis[0][1]=point[set[setNr].pnt[0]].py*scale->w+scale->y;
          v_qdis[0][2]=point[set[setNr].pnt[0]].pz*scale->w+scale->z;
	}
        else
        {
          v_qdis[0][0]=node[set[setNr].node[0]].nx*scale->w+scale->x;
          v_qdis[0][1]=node[set[setNr].node[0]].ny*scale->w+scale->y;
          v_qdis[0][2]=node[set[setNr].node[0]].nz*scale->w+scale->z;
	}
        v_qdis[1][0]=point[set[trgtNr].pnt[0]].px*scale->w+scale->x;
        v_qdis[1][1]=point[set[trgtNr].pnt[0]].py*scale->w+scale->y;
        v_qdis[1][2]=point[set[trgtNr].pnt[0]].pz*scale->w+scale->z;
      }
      else
      {
        if(set[setNr].anz_p==1)
        {
          v_qdis[0][0]=point[set[setNr].pnt[0]].px*scale->w+scale->x;
          v_qdis[0][1]=point[set[setNr].pnt[0]].py*scale->w+scale->y;
          v_qdis[0][2]=point[set[setNr].pnt[0]].pz*scale->w+scale->z;
	}
        else
        {
          v_qdis[0][0]=node[set[setNr].node[0]].nx*scale->w+scale->x;
          v_qdis[0][1]=node[set[setNr].node[0]].ny*scale->w+scale->y;
          v_qdis[0][2]=node[set[setNr].node[0]].nz*scale->w+scale->z;
	}
        v_qdis[1][0]=node[set[trgtNr].node[0]].nx*scale->w+scale->x;
        v_qdis[1][1]=node[set[trgtNr].node[0]].ny*scale->w+scale->y;
        v_qdis[1][2]=node[set[trgtNr].node[0]].nz*scale->w+scale->z;
      }
      da=v_angle(v_qdis[0],v_qdis[1]);
      for(i=0; i<3; i++) { p1[i]=v_qdis[0][i]; p2[i]=v_qdis[1][i]; }
      p1[0]=p2[0]=0.; 
      dax=v_angle(p1,p2);
      for(i=0; i<3; i++) { p1[i]=v_qdis[0][i]; p2[i]=v_qdis[1][i]; }
      p1[1]=p2[1]=0.; 
      day=v_angle(p1,p2);
      for(i=0; i<3; i++) { p1[i]=v_qdis[0][i]; p2[i]=v_qdis[1][i]; }
      p1[2]=p2[2]=0.; 
      daz=v_angle(p1,p2);
      for(i=0; i<3; i++) { p1[i]=v_qdis[0][i]; p2[i]=v_qdis[1][i]; }

      // dist = p1-p2
      dist=sqrt((v_qdis[0][0]-v_qdis[1][0])*(v_qdis[0][0]-v_qdis[1][0])+
      (v_qdis[0][1]-v_qdis[1][1])*(v_qdis[0][1]-v_qdis[1][1])+
      		(v_qdis[0][2]-v_qdis[1][2])*(v_qdis[0][2]-v_qdis[1][2]));
      distx=v_qdis[0][0]-v_qdis[1][0];
      disty=v_qdis[0][1]-v_qdis[1][1];
      distz=v_qdis[0][2]-v_qdis[1][2];
      rp=sqrt(p1[0]*p1[0]+p1[1]*p1[1]+p1[2]*p1[2])-sqrt(p2[0]*p2[0]+p2[1]*p2[1]+p2[2]*p2[2]);
      pr[0]=sqrt(p1[1]*p1[1]+p1[2]*p1[2])-sqrt(p2[1]*p2[1]+p2[2]*p2[2]);
      pr[1]=sqrt(p1[0]*p1[0]+p1[2]*p1[2])-sqrt(p2[0]*p2[0]+p2[2]*p2[2]);
      pr[2]=sqrt(p1[0]*p1[0]+p1[1]*p1[1])-sqrt(p2[0]*p2[0]+p2[1]*p2[1]);
      
      printf("DISTMAX:%lf dxyz:%lf %lf %lf da:%lf daxyz:%lf %lf %lf dr:%lf drxyz:%lf %lf %lf\n",dist,distx,disty,distz,
	   da*180./PI, dax*180./PI, day*180./PI, daz*180./PI,rp,pr[0],pr[1],pr[2]);
	     
      sprintf(parameter[0],"%e",dist);
      sprintf(parameter[1],"%e",distx);
      sprintf(parameter[2],"%e",disty);
      sprintf(parameter[3],"%e",distz);
      sprintf(parameter[4],"%e",da*180./PI);
      sprintf(parameter[5],"%e",dax*180./PI);
      sprintf(parameter[6],"%e",day*180./PI);
      sprintf(parameter[7],"%e",daz*180./PI);
      sprintf(parameter[8],"%e",rp);
      sprintf(parameter[9],"%e",pr[0]);
      sprintf(parameter[10],"%e",pr[1]);
      sprintf(parameter[11],"%e",pr[2]);

      write2stack( 12, parameter);
      return;
    }
    
    /* loop over all points or nodes of the target */
    if(set[setNr].anz_p)
      nrad=&point[set[setNr].pnt[0]].px;
    else if(set[setNr].anz_n)
      nrad=&node[set[setNr].node[0]].nx;
    else return;
    if((set[trgtNr].anz_p)||(set[trgtNr].anz_n))
    {
      for(n=0; n<set[trgtNr].anz_p; n++)
      {
        if(set[trgtNr].anz_p) nx=&point[set[trgtNr].pnt[n]].px;
        else nx=&node[set[trgtNr].node[n]].nx;
        v_result(nx,nrad,pr);
        v_scal(&scale->w,pr,pr );
        dist=v_betrag(pr);
        distx=pr[0];
        disty=pr[1];
        distz=pr[2];
        if(dist>dist_max) { dist_max=dist; pnr1=n; }
        if(dist<dist_min) { dist_min=dist; pnr2=n; }
        if(distx>dist_maxx) { dist_maxx=distx; }
        if(distx<dist_minx) { dist_minx=distx; }
        if(disty>dist_maxy) { dist_maxy=disty; }
        if(disty<dist_miny) { dist_miny=disty; }
        if(distz>dist_maxz) { dist_maxz=distz; }
        if(distz<dist_minz) { dist_minz=distz; }
      }
      
      printf("DISTMAX:%lf dxyz:%e %e %e\n", dist_max, dist_maxx, dist_maxy, dist_maxz );
      sprintf(parameter[0],"%e",dist_max);
      sprintf(parameter[1],"%e",dist_maxx);
      sprintf(parameter[2],"%e",dist_maxy);
      sprintf(parameter[3],"%e",dist_maxz);
      printf("DISTMIN:%lf dxyz:%e %e %e\n", dist_min, dist_minx, dist_miny, dist_minz );
      sprintf(parameter[4],"%e",dist_min);
      sprintf(parameter[5],"%e",dist_minx);
      sprintf(parameter[6],"%e",dist_miny);
      sprintf(parameter[7],"%e",dist_minz);
    }

    write2stack( 8, parameter);
    return;
  }

  /* cycle through all entities of setNr and add them to the special set  */
  /* cyrcle through all bodys and add  */
  if( (setdep=pre_seta( specialset->dep, "i", 0 )) <0 ) return;
  for (i=0; i<set[setNr].anz_n; i++)
  {
    seta( setdep, "n", set[setNr].node[i] );
  }
  /* add all related elements for a later midside-node-correction */
  completeSet( specialset->dep, "up") ;

  for (i=0; i<set[setNr].anz_b; i++)
  {
    seta( setdep, "b", set[setNr].body[i] );
  }
  /* cyrcle through all surfs and add  */
  for (i=0; i<set[setNr].anz_s; i++)
  {
    seta( setdep, "s", set[setNr].surf[i] );
  }
  /* cyrcle through all lcmbs and add  */
  for (i=0; i<set[setNr].anz_c; i++)
  {
    seta( setdep, "c", set[setNr].lcmb[i] );
  }
  /* cyrcle through all lines and add  */
  for (i=0; i<set[setNr].anz_l; i++)
  {
    seta( setdep, "l", set[setNr].line[i] );
  }
  for (i=0; i<set[setNr].anz_p; i++)
  {
    seta( setdep, "p", set[setNr].pnt[i] );
  }
  /* second cycle through all entities and add lower ones  to the special set  */
  // completeSet( specialset->dep, "do") ;


  if (set[setNr].anz_n==0) set[setdep].anz_n=set[setNr].anz_n;
  setNr=setdep;
  trgtNr=setind;

  /* generate an array of all points or nodes to be projected */
  /* the original coordinates must be used for radial projection to get the correct axis */
  sum_pproj=0;
  for (i=0; i<set[setNr].anz_p; i++)
  {
    p=set[setNr].pnt[i];
    if(point[p].name==(char *)NULL) continue;
    if (p<0)
    {
      errMsg(" ERROR: Point-nr:%d is undefined\n", set[setNr].pnt[i] );
      goto nextpoint;
    }
    if ( (pproj = (Points *)realloc( (Points *)pproj, (sum_pproj+2) * sizeof(Points))) == NULL )
    {
      errMsg("\nERROR: realloc failed in pre_proj() \n\n");
    }
    pproj[sum_pproj].nn=p;
    pproj[sum_pproj].name=point[p].name;
    pproj[sum_pproj].px=point[p].px*scale->w+scale->x;
    pproj[sum_pproj].py=point[p].py*scale->w+scale->y;
    pproj[sum_pproj].pz=point[p].pz*scale->w+scale->z;
    sum_pproj++;
  }
  for (i=0; i<set[setNr].anz_n; i++)
  {
    n=set[setNr].node[i];
    if ( (pproj = (Points *)realloc( (Points *)pproj, (sum_pproj+2) * sizeof(Points))) == NULL )
    {
      errMsg("\nERROR: realloc failed in pre_proj() \n\n");
    }
    pproj[sum_pproj].nn=-n;
    sprintf(name,"%d",n);
    if((pproj[sum_pproj].name= (char *)malloc((strlen(name)+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed\n\n" ); return; }
    strcpy(pproj[sum_pproj].name,name);
    pproj[sum_pproj].px=node[n].nx*scale->w+scale->x;
    pproj[sum_pproj].py=node[n].ny*scale->w+scale->y;
    pproj[sum_pproj].pz=node[n].nz*scale->w+scale->z;
    sum_pproj++;
  }
  if(!sum_pproj) return;

  /* fill all unfilled surfaces, do it in any case because points could have been moved! */
  /* repShape will change *scale and therefore must be executed first */
  repShape(trgtNr);
  for (j=0; j<set[trgtNr].anz_s; j++)
  {
    //orientSet( set[trgtNr].name );
    if(surf[set[trgtNr].surf[j]].sh>-1)
    {
      if(shape[surf[set[trgtNr].surf[j]].sh].type==4)
      {
        repNurs(shape[surf[set[trgtNr].surf[j]].sh].p[0]); untrimNurs(shape[surf[set[trgtNr].surf[j]].sh].p[0]);
      }
    }
    repSurf(set[trgtNr].surf[j],1);
  }

  /* generate splitting triangles */
  sum_tri=genSplitTrias(trgtNr, &tri, 1);

  /* if no surfs and the type is proj and the direction nor then project directly to the shapes (offset in this case not regarded) */
  if((functionFlag)&&(!sum_tri))
  {
    if((compare(action, "nor",3) == 3)&&(set[trgtNr].anz_sh))
    {
      for(i=0; i<set[trgtNr].anz_sh; i++)
      {
        if(shape[set[trgtNr].shp[i]].type==4) projSetToNurbs( shape[set[trgtNr].shp[i]].p[0], set, setNr, point, &node);
        else
	{
	  Stmp=-1;
	  if((shape[set[trgtNr].shp[i]].type==1)||(shape[set[trgtNr].shp[i]].type==2)) Stmp=coneToNurs(set[trgtNr].shp[i], 0);
	  if(shape[set[trgtNr].shp[i]].type==3) Stmp=sphToNurs(set[trgtNr].shp[i], 0);
	  if(shape[set[trgtNr].shp[i]].type==5) Stmp=torusToNurs(set[trgtNr].shp[i], 0);
	  if(Stmp>-1)
	  {
	    projSetToNurbs( Stmp, set, setNr, point, &node);
            for (j=0; j<nurbs[Stmp].u_npnt; j++) delPnt( nurbs[Stmp].v_npnt, nurbs[Stmp].ctlpnt[j] );
            //seta( setall, "S", Stmp );
            delNurs( 1, &Stmp );
	  }
	}
      }
      goto skipTriProj;
    }
    else if( compare( action, "rad", 3) == 3) ;  // will be executed furher down
    else
    {
      printf(" ERROR: Transformation type '%s' not implemented for shapes (only 'nor'), please use a surface.\n", action);
      goto skipTriProj;
    }
  }

  /*
  printf("sum_tri:%d\n",sum_tri);
  j=1; for( n=0; n<sum_tri; n++)
  {
    printf(" node %d %f %f %f\n", j++, tri[n].p1[0], tri[n].p1[1], tri[n].p1[2]);
    printf(" node %d %f %f %f\n", j++, tri[n].p2[0], tri[n].p2[1], tri[n].p2[2]);
    printf(" node %d %f %f %f\n", j++, tri[n].p3[0], tri[n].p3[1], tri[n].p3[2]);
    printf(" elem %d %d %d %d tr3\n", n+1, j-3,j-2,j-1);
  }
  */

  /* stelle daten fuer near3d bereit */
  n_closest_tri=sum_tri;

  if ( (near_tri = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
    printf("ERROR: realloc failed: Rsort\n\n" ); 
  if ( (orig_x = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_y = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_z = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_x = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_y = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_z = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_nx = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ny = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_nz = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if( compare( action, "rot", 3) != 3)
  {
  for(i=0; i<sum_tri; i++)
  {
    rsort[i].r=orig_x[i]=tri[i].cg[0];
    rsort[i].i=i;
  }
  qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<sum_tri; i++)
  {
    sort_x[i]=rsort[i].r;
    sort_nx[i]=rsort[i].i;
  }
  for(i=0; i<sum_tri; i++)
  {
    rsort[i].r=orig_y[i]=tri[i].cg[1];
    rsort[i].i=i;
  }
  qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<sum_tri; i++)
  {
    sort_y[i]=rsort[i].r;
    sort_ny[i]=rsort[i].i;
  }
  for(i=0; i<sum_tri; i++)
  {
    rsort[i].r=orig_z[i]=tri[i].cg[2];
    rsort[i].i=i;
  }
  qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<sum_tri; i++)
  {
    sort_z[i]=rsort[i].r;
    sort_nz[i]=rsort[i].i;
  }
  }

  /* project the nodes and points */
  if(getSetNr(specialset->noprj)>-1) delSet(specialset->noprj);
  if( compare( action, "rad", 3) == 3)
  {
    offset=0.;
    mindistini=MAX_FLOAT;
    length = sscanf( record,"%*s %*s %*s %c %lf %lf", &axis, &offset, &mindistini );

    /* target needs lines or triangles */
    if( sum_tri == 0) 
    {
      if( set[trgtNr].anz_l > 0)
      {
        surfFlag=0;
        for (j=0; j<set[trgtNr].anz_l; j++) repLine(set[trgtNr].line[j]);
      }
      else
      {
        errMsg(" ERROR: Set (%s) does not have lines, surfaces or a triangle-mesh\n", targetname );
        return;
      }
    }
    else 
    {
      surfFlag=1;
    }

    for (i=0; i<sum_pproj; i++)
    {
      /* the original coordinates must be used for radial projection to get the correct axis */
      pr[0]=pproj[i].px;
      pr[1]=pproj[i].py;
      pr[2]=pproj[i].pz;

      /* move the point to the line of nodes or to the surface */
      if( surfFlag)
      {
        /* The embedded triangles of the surfaces are used as a target for the projection */
        /* determine the position where the radial-ray through the point meets the triangles */
        /* and determine if the position is inside the triangle */

        if( axis=='x')
        {
          vproj[0]=0.;
          vproj[1]=pr[1];
          vproj[2]=pr[2];
	}
        if( axis=='y')
        {
          vproj[0]=pr[0];
          vproj[1]=0.;
          vproj[2]=pr[2];
	}
        if( axis=='z')
        {
          vproj[0]=pr[0];
          vproj[1]=pr[1];
          vproj[2]=0.;
	}

        counter=0;
        do
        {
	  // prevent superfluous loops
	  if(pow(10.,counter)>n_closest_tri) { printf("counter %d\n",counter+1); break;}
	  
          counter++;
	  min_n_closest_tri=pow(10.,counter);
          curr_n_closest_tri=n_closest_tri*pow(2,counter-PROJ_LOOPS);
          if(curr_n_closest_tri<min_n_closest_tri) { curr_n_closest_tri=min_n_closest_tri; if(curr_n_closest_tri>n_closest_tri) curr_n_closest_tri=n_closest_tri; }

          /* find the closest tri */
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, pr[0], pr[1], pr[2],
                sum_tri, &near_tri[0], curr_n_closest_tri);
          /*
          printf("n_closest_tri:%d\n", n_closest_tri);
          for(j=0; j<10; j++)
          {
            n=near_tri[j];
            printf("tri n:%d fnr:%d %f %f %f\n", n, tri[n].fnr, tri[n].cg[0], tri[n].cg[1] , tri[n].cg[2]);
            printf("dist: %f %f %f\n", tri[n].cg[0]-pr[0], tri[n].cg[1]-pr[1], tri[n].cg[2]-pr[2]);
            printf("dist: %f\n", sqrt((tri[n].cg[0]-pr[0])*(tri[n].cg[0]-pr[0])+ (tri[n].cg[1]-pr[1])*(tri[n].cg[1]-pr[1])+ (tri[n].cg[2]-pr[2])*(tri[n].cg[2]-pr[2])));
          }
          */
  
          /* check the closest tri for a split-point */
          dist=MAX_FLOAT; minj=-1; mindist=mindistini; triScale=1.;
	  v_norm( vproj, eg );
          do
          {
            for(j=0; j<curr_n_closest_tri; j++)
            {
              n=near_tri[j];
              v_result( tri[n].p1, tri[n].p2, p01 );
              v_norm( p01, eu );
              v_result( tri[n].p1, tri[n].p3, p01 );
              v_norm( p01, ev );
              v_prod( eu, ev, p01);
              v_norm( p01, en );
              // bestimme den Abstand zwischen den Aufpunkten der Linie und Ebene 
              v_result( pr, tri[n].p1, p01 );
              g = AsplitL( p01, eu, ev, eg, en );
	      if(g==MAX_FLOAT) continue;
              v_scal( &g, eg, p01 );
	      // dist between point and split point
	      dist=v_betrag(p01);
              v_add( pr, p01, psplt );
              distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, psplt, en, triScale, &orient);
              if(( distn != OUTSIDE )&&(abs(dist)<abs(mindist)))
              {
                if(printFlag)
		  printf("counter:%d found enclosing tri3:%d for point:%s, distance:%lf %lf\n", counter, n, pproj[i].name, dist, mindist); 
                mindist=dist;
                minj=j;
              }
            }
            if( minj!=-1 ) break;
            else triScale+=0.1;
          }while(triScale<=PROJ_BIAS);
  
          if( minj!=-1 )
          {
            j=minj;
            n=near_tri[j];
            dist=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pr, vproj, 100., &orient);
            /* berechne den Gradeneinheitsvektor en */
            v_norm( vproj, pc );
            pr[0]+=pc[0]*(dist+offset);
            pr[1]+=pc[1]*(dist+offset);
            pr[2]+=pc[2]*(dist+offset);
	    if(dist != OUTSIDE) break;
          }
	  //}while( ((dist == OUTSIDE) || (dist>gtol))&&(counter<PROJ_LOOPS));
	}while( (distn == OUTSIDE)&&(counter<PROJ_LOOPS));

        if(minj!=-1 )
        {
          if(printFlag)
            printf(" found enclosing tri3[%d] for point:%s, distance:%lf triScale:%f break at j=%d  loop:%d\n", n, pproj[i].name, dist, triScale,minj, counter); 
        }
        else
        {
          if(pproj[i].nn>-1) { printf("WARNING: no enclosing element for point:%s found\n", pproj[i].name ); pre_seta(specialset->noprj, "p",pproj[i].name ); } 
          else               { printf("WARNING: no enclosing element for node:%s found\n", pproj[i].name );  pre_seta(specialset->noprj, "n",pproj[i].name ); }
          goto nextpoint;
        }
      }
      else
      {
        /* Internal points of the target-lines are used as a target for the projection */

        /* count the internal points */
        for (j=0; j<set[trgtNr].anz_l; j++)
        {
          anz_n+=line[set[trgtNr].line[j]].nip;
	}
	anz_n/=3;

        if ( (nx = (double *)malloc( (anz_n+1) * sizeof(double))) == NULL )
        {  errMsg("\n\n ERROR: malloc failed in pre_proj()\n") ; free_proj(nx, nrad, nmissed); return; }
        if ( (nrad = (double *)malloc( (anz_n+1) * sizeof(double))) == NULL )
        {  errMsg("\n\n ERROR: malloc failed in pre_proj()\n") ; free_proj(nx, nrad, nmissed); return; }

        if( axis=='x') j=0;
        else if( axis=='y') j=1;
        else if( axis=='z') j=2;
        else {  errMsg("\n\n ERROR: axis unknown in pre_proj()\n") ; free_proj(nx, nrad, nmissed); return; };
        if( v_rec2cyl( pr, j, csys, pc) <0) goto nextpoint;

        /* move the point to the target-line */
        e=0;
        for (j=0; j<set[trgtNr].anz_l; j++)
        {
          for (n=0; n<line[set[trgtNr].line[j]].nip; n+=3)
          {
            nr[0]=line[set[trgtNr].line[j]].ip[n]*scale->w+scale->x;
            nr[1]=line[set[trgtNr].line[j]].ip[n+1]*scale->w+scale->y;
            nr[2]=line[set[trgtNr].line[j]].ip[n+2]*scale->w+scale->z;
            nx[e]=nr[csys[0]];
            nrad[e++]=sqrt(nr[csys[1]]*nr[csys[1]]+nr[csys[2]]*nr[csys[2]]);
	    //printf(" target-line %f %f\n", nx[e-1], nrad[e-1]);
          }
        }
        //printf("move the point to the target-line\n");
        mode=INTPOLMODE; 
        pc[0]=intpol2( nx, nrad, e, pc[2], &mode )+offset;
        
	//printf(" csys %d %d %d\n", csys[0], csys[1],csys[2]);
        pr[csys[0]]=pc[2];
        pr[csys[1]]=cos(pc[1])*pc[0];
        pr[csys[2]]=sin(pc[1])*pc[0];
      }
  
      if(functionFlag)
      {
        /* redefine the point */
        pproj[i].px= pr[0];
        pproj[i].py= pr[1];
        pproj[i].pz= pr[2];
      }
      else
      {
        v_result(&pproj[i].px, pr, p1p2);
	//printf(" pp %f %f %f\n", pproj[i].px,pproj[i].py,pproj[i].pz);
	//printf(" pr %f %f %f\n", pr[0], pr[1],pr[2]);

        dist=v_betrag(p1p2);
        // since the distance should be given as distance from target:
        if(v_betrag(pr)>v_betrag(&pproj[i].px)) dist*=-1;
        if(dist>dist_max) dist_max=dist;
        if(dist<dist_min) dist_min=dist;
        sum_dist+=dist;
        ndist++;
        //if(printFlag) printf("dist:%f av:%f ndist:%d max:%f min:%f\n", dist, sum_dist/ndist, ndist, dist_max, dist_min);
      }

    nextpoint:;
    }
  }
  else if( compare( action, "rot", 3) == 3)
  {
    offset=0.;
    mindistini=MAX_FLOAT;
    length = sscanf( record, "%*s%*s%*s%s%s%lf%lf", pkt1, pkt2, &offset, &mindistini );

    p1[0] = 0.;
    p1[1] = 0.;
    p1[2] = 0.;
    p2[0] = 0.;
    p2[1] = 0.;
    p2[2] = 0.;

    if(!length) 
    {
      errMsg(" ERROR: not enough parameter specified\n");
      return;
    }
    else if(length==1)
    {
      if(pkt1[0]=='x')   p2[0] = 1.;
      else if(pkt1[0]=='y')   p2[1] = 1.;
      else if(pkt1[0]=='z')   p2[2] = 1.;
      else
      {
        errMsg(" Axis:%s is undefined\n", pkt1 );
        return;
      }
      offset=0.;
    }
    else if(length>=2)
    {
      pnr1=getPntNr( pkt1 );
      pnr2=getPntNr( pkt2 );
      if(pnr2<0)
      {
        if(pkt1[0]=='x')   p2[0] = 1.;
        else if(pkt1[0]=='y')   p2[1] = 1.;
        else if(pkt1[0]=='z')   p2[2] = 1.;
        else
        {
          errMsg(" Axis:%s is undefined\n", pkt1 );
          return;
        }
        offset=atof(pkt2);
      }
      else
      {
        p1[0] = point[pnr1].px*scale->w+scale->x;
        p1[1] = point[pnr1].py*scale->w+scale->y;
        p1[2] = point[pnr1].pz*scale->w+scale->z;
    
        p2[0] = point[pnr2].px*scale->w+scale->x;
        p2[1] = point[pnr2].py*scale->w+scale->y;
        p2[2] = point[pnr2].pz*scale->w+scale->z;
      }
    }
    if(offset!=0)
    {
      printf("\n WARNING option offset not implemented so far. offset:%lf ignored.\n\n",offset);
    }

    /* The embedded triangles are used as a target for the projection */

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

    /* Berechnung der lhq-koordinaten des Startpunktes der Drehachse (pnr1 ist der 0-Punkt des lhq systems) */
    x=p1[0];
    y=p1[1];
    z=p1[2];
    
    l_offs=ex[0]*x+ey[0]*y+ez[0]*z;
    h_offs=ex[1]*x+ey[1]*y+ez[1]*z;
    q_offs=ex[2]*x+ey[2]*y+ez[2]*z;
    /*
    printf("el:%lf %lf %lf eh:%lf %lf %lf eq:%lf %lf %lf\n", el[0],el[1],el[2], eh[0],eh[1],eh[2], eq[0],eq[1],eq[2]);
    printf("ex:%lf %lf %lf ey:%lf %lf %lf ez:%lf %lf %lf\n", ex[0],ex[1],ex[2], ey[0],ey[1],ey[2], ez[0],ez[1],ez[2]);
    printf("xyz p1:%lf %lf %lf\n", x,y,z);
    printf("lhq p1:%lf %lf %lf\n", l_offs, h_offs, q_offs);
    */

    /* umrechnen der tri schwerpunkte auf lhq cylsys */
    for(i=0; i<sum_tri; i++)
    {
      x=tri[i].cg[0];
      y=tri[i].cg[1];
      z=tri[i].cg[2];
      l=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;   /* l (drehachse) */
      h=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;   /* h */
      q=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;   /* q */
      tri[i].cg[0]=sqrt(h*h+q*q);
      tri[i].cg[1]=p_angle(h,q);
      tri[i].cg[2]=l;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_x[i]=tri[i].cg[0];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_x[i]=rsort[i].r;
      sort_nx[i]=rsort[i].i;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_y[i]=tri[i].cg[1];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_y[i]=rsort[i].r;
      sort_ny[i]=rsort[i].i;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_z[i]=tri[i].cg[2];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_z[i]=rsort[i].r;
      sort_nz[i]=rsort[i].i;
    }

    /* Berechnung der lhq-koordinaten aller zu drehenden punkte */
    for (i=0; i<sum_pproj; i++)
    {
      x=pproj[i].px;
      y=pproj[i].py;
      z=pproj[i].pz;

      ppt[0]=pp[0]=l=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;   /* l (drehachse) */
      pp[1]=h=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;   /* h */
      pp[2]=q=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;   /* q */
      r=sqrt(h*h+q*q);
      fi=p_angle(h,q);

      /* define a cutting plane pa by 3 points ( pa1,pa2,pa3) in the lhq system */
      pa1[0]=l;  /* Attention: =l is not a 1 ('one'), its an lowercase 'L' (the 'l' coordinate from lhq) */
      pa1[1]=0.;
      pa1[2]=0.;
      pa2[0]=l;
      pa2[1]=1.;
      pa2[2]=0.;
      pa3[0]=l;
      pa3[1]=0.;
      pa3[2]=1.;

      counter=0;
      do
      {
	// prevent superfluous loops
	if(pow(10.,counter)>n_closest_tri) { printf("counter %d\n",counter+1); break;}
	  
        counter++;
        min_n_closest_tri=pow(10.,counter);
        curr_n_closest_tri=n_closest_tri*pow(2,counter-PROJ_LOOPS);
        if(curr_n_closest_tri<min_n_closest_tri) { curr_n_closest_tri=min_n_closest_tri; if(curr_n_closest_tri>n_closest_tri) curr_n_closest_tri=n_closest_tri; }

        /* find the closest tri */
        near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, r,fi,l, sum_tri, &near_tri[0], curr_n_closest_tri);

        /* check the closest tri for a split-point */
        for(j=0; j<curr_n_closest_tri; j++)
        {
          n=near_tri[j];
          x=tri[n].p1[0];
          y=tri[n].p1[1];
          z=tri[n].p1[2];
          pt1[0]=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;    /* l */
          pt1[1]=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;    /* h */
          pt1[2]=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;    /* q */
  
          x=tri[n].p2[0];
          y=tri[n].p2[1];
          z=tri[n].p2[2];
          pt2[0]=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;    /* l */
          pt2[1]=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;    /* h */
          pt2[2]=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;    /* q */
  
          x=tri[n].p3[0];
          y=tri[n].p3[1];
          z=tri[n].p3[2];
          pt3[0]=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;    /* l */
          pt3[1]=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;    /* h */
          pt3[2]=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;    /* q */
  
          dist=OUTSIDE;
          if( AsplitA( pa1, pa2, pa3, pt1, pt2, pt3, ps1, ps2) >0 )
          {
            /* check if the projected point is on that element */
	    /*
            printf("seto tri_%d_%d_%d\n",i,j,counter);
            printf("seto pp\n");
	    printf("pnt pp_%d_%d_%d %lf %lf %lf\n",i,j,counter,pp[0],pp[1],pp[2]);
            printf("setc pp\n");
    	    printf("pnt ps1_%d_%d_%d %lf %lf %lf\n",i,j,counter,ps1[0],ps1[1],ps1[2]);
  	    printf("pnt ps2_%d_%d_%d %lf %lf %lf\n",i,j,counter,ps2[0],ps2[1],ps2[2]);
  	    printf("pnt pa1_%d_%d_%d %lf %lf %lf\n",i,j,counter,counter,pa1[0],pa1[1],pa1[2]);
  	    printf("pnt pa2_%d_%d_%d %lf %lf %lf\n",i,j,counter,pa2[0],pa2[1],pa2[2]);
  	    printf("pnt pa3_%d_%d_%d %lf %lf %lf\n",i,j,counter,pa3[0],pa3[1],pa3[2]);
            printf("seto pt\n");
  	    printf("pnt pt1_%d_%d_%d %lf %lf %lf\n",i,j,counter,pt1[0],pt1[1],pt1[2] );
  	    printf("pnt pt2_%d_%d_%d %lf %lf %lf\n",i,j,counter,pt2[0],pt2[1],pt2[2] );
  	    printf("pnt pt3_%d_%d_%d %lf %lf %lf\n",i,j,counter,pt3[0],pt3[1],pt3[2] );
            printf("setc pt\n");
            printf("setc tri_%d_%d_%d\n",i,j,counter);
	    */
  
            v_result(ps1,ps2,e1);
            rp=v_norm( e1, e1 );
            /* check if ps1==ps2 then no intersection line could be determined and no projection is possible */
            if(rp>1.e-10)
  	    {
              pval=2*(ps1[1]*e1[1]+ps1[2]*e1[2])/(e1[1]*e1[1]+e1[2]*e1[2]);
              qval=ps1[1]*ps1[1]+ps1[2]*ps1[2]-pp[1]*pp[1]-pp[2]*pp[2];
              divpq=pval*pval*0.25-qval;
              if(divpq>=0.)
  	      {
                if(printFlag) printf(" found enclosing tri3 for point:%s\n", pproj[i].name);
    
                /* new h and q values  */
                scal_e[0]=-pval*0.5 +sqrt(divpq);
                scal_e[1]=-pval*0.5 -sqrt(divpq);
                //printf("p:%f q:%f scal:%f scal:%f\n", pval, qval, scal_e[0], scal_e[1]); 
                v_scal(&scal_e[0], e1, p1);
                v_add(ps1,p1, p1);
                v_scal(&scal_e[1], e1, p2);
                v_add(ps1,p2, p2);
              /*
                printf("pnt e1: %lf %lf %lf\n",e1[0],e1[1],e1[2] );
                printf("pnt pc1: %lf %lf %lf\n",p1[0],p1[1],p1[2] );
                printf("pnt pc2: %lf %lf %lf\n",p2[0],p2[1],p2[2] );
              */  
                /* check which solution gives the smallest correction */
                v_result(pp,p1,v1);
                v_result(pp,p2,v2);
                //printf("v1:%f v2:%f\n",v_betrag(v1),v_betrag(v2)); 
                bv1=v_betrag(v1);
                bv2=v_betrag(v2);
  
                if(bv1<bv2)
                {
                  dist=bv1;
                  h=p1[1];
                  q=p1[2];
                }
                else
                {
                  dist=bv2;
                  h=p2[1];
                  q=p2[2];
                }
                /* check if the projected point is inside the tri */
                ppt[1]=h;
                ppt[2]=q;
		//printf("ppt hq:%f %f ps1:%f %f ps2:%f %f dps:%e %e\n",ppt[1],ppt[2],ps1[1],ps1[2],ps2[1],ps2[2], ps2[1]-ps1[1],ps2[2]-ps1[2]);
		//if( ((ppt[1]<=ps1[1])&&(ppt[1]>=ps2[1]))||((ppt[1]>=ps1[1])&&(ppt[1]<=ps2[1]))) printf("inside h\n");
		//if( ((ppt[2]<=ps1[2])&&(ppt[2]>=ps2[2]))||((ppt[2]>=ps1[2])&&(ppt[2]<=ps2[2]))) printf("inside q\n");
                //if(check_tri3(ppt,pt1,pt2,pt3,1)==MAX_INTEGER) dist=OUTSIDE;
                if(check_line(ppt,ps1,ps2,1)==MAX_INTEGER) dist=OUTSIDE;
                else if(abs(dist)<abs(mindistini)) goto foundTri3rot;
              }
            }
          }
        }
	//}while( ((dist == OUTSIDE) || (dist>gtol))&&(counter<PROJ_LOOPS));
      }while( (dist == OUTSIDE)&&(counter<PROJ_LOOPS));
      if(pproj[i].nn>-1)
      { printf("WARNING: no enclosing element for point:%s found\n", pproj[i].name ); pre_seta(specialset->noprj, "p",pproj[i].name ); } 
      else
      { printf("WARNING: no enclosing element for node:%s found\n", pproj[i].name );  pre_seta(specialset->noprj, "n",pproj[i].name ); }
      goto nextpointrot;
     foundTri3rot:;
      
      lhq[0]=l;
      lhq[1]=h;
      lhq[2]=q;
      l+= l_offs;
      h+= h_offs;
      q+= q_offs;
      
      x=el[0]*l+eh[0]*h+eq[0]*q;
      y=el[1]*l+eh[1]*h+eq[1]*q;
      z=el[2]*l+eh[2]*h+eq[2]*q;
  
      if(functionFlag)
      {
        /* redefine the point */
        pproj[i].px=  x;
        pproj[i].py=  y;
        pproj[i].pz=  z;
      }
      else
      {
        pp[0]=lhq[0]=0.;
	angle=v_angle(pp,lhq);

        // generate a signum by try and error
        h=lhq[1]*cos(-angle)-lhq[2]*sin(-angle);
        q=lhq[1]*sin(-angle)+lhq[2]*cos(-angle);
        if((abs(h-pp[1])>1.e-6)||(abs(q-pp[2])>1.e-6)) angle*=-1.;
        dist=angle*180./PI;
        // since the distance should be given as distance from target:
        dist*=-1;
        if(dist>dist_max) dist_max=dist;
        if(dist<dist_min) dist_min=dist;
        sum_dist+=dist;
        ndist++;
	if(printFlag) printf("dist:%f av:%f ndist:%d max:%f min:%f\n", dist, sum_dist/ndist, ndist, dist_max, dist_min);
      }
     nextpointrot:;
    }
  }
  else if(( compare( action, "tra", 3) == 3)||( compare( action, "nor", 3) == 3))
  {
    offset=0.;
    mindistini=MAX_FLOAT;
    if( compare( action, "tra", 3) == 3)
      length = sscanf( record,"%*s %*s %*s %lf %lf %lf %lf %lf", &vproj[0], &vproj[1], &vproj[2], &offset, &mindistini );
    else
    {
      length = sscanf( record,"%*s %*s %*s %lf %lf", &offset, &mindistini);
      normFlag=1;
    }

    /* The embedded triangles are the surfaces which are used as a target for the projection */
    for (i=0; i<sum_pproj; i++)
    {
      pr[0]=pproj[i].px;
      pr[1]=pproj[i].py;
      pr[2]=pproj[i].pz;

      counter=0;
      n=-1;
      do
      {
        // prevent superfluous loops
        if(pow(10.,counter)>n_closest_tri) { printf("counter %d\n",counter+1); break;}
        distn = OUTSIDE;
	
        counter++;
        min_n_closest_tri=pow(10.,counter);
        curr_n_closest_tri=n_closest_tri*pow(2,counter-PROJ_LOOPS);
        if(curr_n_closest_tri<min_n_closest_tri) { curr_n_closest_tri=min_n_closest_tri; if(curr_n_closest_tri>n_closest_tri) curr_n_closest_tri=n_closest_tri; }

        /* find the closest tri */
        near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, pr[0], pr[1], pr[2],
              sum_tri, &near_tri[0], curr_n_closest_tri);
        /*
        printf("n_closest_tri:%d\n", n_closest_tri);
        for(j=0; j<n_closest_tri; j++)
        {
          n=near_tri[j];
          printf("tri n:%d fnr:%d %f %f %f\n", n, tri[n].fnr, tri[n].cg[0], tri[n].cg[1] , tri[n].cg[2]);
          printf("dist: %f %f %f\n", tri[n].cg[0]-pr[0], tri[n].cg[1]-pr[1], tri[n].cg[2]-pr[2]);
          printf("dist: %f\n", sqrt((tri[n].cg[0]-pr[0])*(tri[n].cg[0]-pr[0])+ (tri[n].cg[1]-pr[1])*(tri[n].cg[1]-pr[1])+ (tri[n].cg[2]-pr[2])*(tri[n].cg[2]-pr[2])));
        }
        */

        /* check the closest tri for a split-point */
        dist=MAX_FLOAT; minj=-1;mindist=mindistini; triScale=1.;
        if(!normFlag) v_norm( vproj, eg );
        do
        {
          for(j=0; j<curr_n_closest_tri; j++)
          {
            n=near_tri[j];
            if(normFlag)
   	    {
              v_result( tri[n].p1, tri[n].p2, v1);
              v_result( tri[n].p1, tri[n].p3, v2);
              v_prod( v1, v2, vproj );
              v_norm( vproj, en );
              dist=distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pr, en, triScale, &orient);
   	    }
	    else
	    {
              v_result( tri[n].p1, tri[n].p2, p01 );
              v_norm( p01, eu );
              v_result( tri[n].p1, tri[n].p3, p01 );
              v_norm( p01, ev );
              v_prod( eu, ev, p01);
              v_norm( p01, en );
              // bestimme den Abstand zwischen den Aufpunkten der Linie und Ebene 
              v_result( pr, tri[n].p1, p01 );
              g = AsplitL( p01, eu, ev, eg, en );
	      if(g==MAX_FLOAT) continue;
              v_scal( &g, eg, p01 );
	      // dist between point and split point
	      dist=v_betrag(p01);
              v_add( pr, p01, psplt );
              distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, psplt, en, triScale, &orient);
	    }
            //printf(" tri3:%d for point:%s dist:%f mindist:%f triScale:%f\n", n, pproj[i].name, dist, mindist, triScale); 
            if(( distn != OUTSIDE )&&(abs(dist)<abs(mindist)))
            {
              if(printFlag)
		printf("counter:%d found enclosing tri3:%d for point:%s, distance:%lf %lf\n", counter, n, pproj[i].name, dist, mindist); 
              mindist=dist;
              minj=j;
            }
          }
          if( minj!=-1 ) break;
          else triScale+=0.01;
        }while(triScale<=PROJ_BIAS);
  
        if( minj!=-1 )
        {
          j=minj;
          n=near_tri[j];
          if(normFlag)
  	  {
            v_result( tri[n].p1, tri[n].p2, v1);
            v_result( tri[n].p1, tri[n].p3, v2);
            v_prod( v1, v2, vproj );
  	  }
          dist=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pr, vproj, 100., &orient);
	  if(dist != OUTSIDE) break;
        }
	//}while( ((dist == OUTSIDE) || (dist>gtol))&&(counter<PROJ_LOOPS));
      }while( (distn == OUTSIDE)&&(counter<PROJ_LOOPS));

      if(minj!=-1 )
      {
	if(n==-1) { printf(" ERROR: found no enclosing tri3. This should not happen, talk to the programmer.\n"); return; }
        if(printFlag)
          printf(" found enclosing tri3[%d] for point:%s, distance:%lf triScale:%f break at j=%d  loop:%d\n", n, pproj[i].name, dist, triScale,minj, counter); 
        if(functionFlag)
	{
          /* berechne den Gradeneinheitsvektoren */
          v_norm( vproj, pc );
          pr[0]+=pc[0]*(dist+offset);
          pr[1]+=pc[1]*(dist+offset);
          pr[2]+=pc[2]*(dist+offset);

          /* redefine the point */
          pproj[i].px= pr[0];
          pproj[i].py= pr[1];
          pproj[i].pz= pr[2];
	}
        else
	{
          /* berechne den Gradeneinheitsvektoren */
          /*          
          v_norm( vproj, pc );
          pr[0]=pc[0]*(dist+offset);
          pr[1]=pc[1]*(dist+offset);
          pr[2]=pc[2]*(dist+offset);
          dist=v_betrag(pr);
	  */
          // instead keep the sign:
          dist=dist-offset;
          // since the distance should be given as distance from target:
          dist*=-1;
          if(dist>dist_max) dist_max=dist;
          if(dist<dist_min) dist_min=dist;
          sum_dist+=dist;
          ndist++;
	  if(printFlag) printf("dist:%f av:%f ndist:%d max:%f min:%f\n", dist, sum_dist/ndist, ndist, dist_max, dist_min);
	}
      }
      else
      {
        if(pproj[i].nn>-1) { printf("WARNING: no enclosing element for point:%s found\n", pproj[i].name ); pre_seta(specialset->noprj, "p",pproj[i].name ); } 
        else               { printf("WARNING: no enclosing element for node:%s found\n", pproj[i].name );  pre_seta(specialset->noprj, "n",pproj[i].name ); }
      }
    }
  }
  else
  {
    errMsg(" ERROR: transformation:%s not known\n", action);
    return;
  }

  /* map the results to the original entities */
  if(functionFlag) for (i=0; i<sum_pproj; i++)
  {
    if(pproj[i].nn>-1)
    {
      p=pproj[i].nn;
      point[p].px=(pproj[i].px-scale->x)/scale->w;
      point[p].py=(pproj[i].py-scale->y)/scale->w;
      point[p].pz=(pproj[i].pz-scale->z)/scale->w;
    }
    else
    {
      n=-pproj[i].nn;
      node[n].nx=(pproj[i].px-scale->x)/scale->w;
      node[n].ny=(pproj[i].py-scale->y)/scale->w;
      node[n].nz=(pproj[i].pz-scale->z)/scale->w;
    }
  }
  free(pproj);
  free(tri);
  free(rsort);
  if(sum_tri) free(near_tri);
  if(orig_x) free(orig_x);
  if(orig_y) free(orig_y);
  if(orig_z) free(orig_z);
  if(sort_x) free(sort_x);
  if(sort_y) free(sort_y);
  if(sort_z) free(sort_z);
  if(sort_nx) free(sort_nx);
  if(sort_ny) free(sort_ny);
  if(sort_nz) free(sort_nz);

  if(getSetNr(specialset->noprj)>-1)
  {
    printf("WARNING: check set:%s for failed entities\n", specialset->noprj);
    if(( compare( action, "nor", 3) == 3)&&(set[trgtNr].anz_sh))
    {
      /* at last project normal onto the referenced nurbs-surfaces */

      printf("  trying to project failed entities to nurbs-shapes (successfull proj not removed from %s)\n", specialset->noprj);
      setNr=getSetNr(specialset->noprj);
      for (i=0; i<set[trgtNr].anz_sh; i++)
      {
        if(shape[set[trgtNr].shp[i]].type==4)
          projSetToNurbs( shape[set[trgtNr].shp[i]].p[0], set, setNr, point, &node);
      }

      if(getSetNr(specialset->zap)>-1) delSet(specialset->zap);
    }
  }

  if(!functionFlag)
  {
    printf("DIST:%f MAX:%f MIN:%f\n", sum_dist/ndist, dist_max, dist_min);
    free_proj(nx, nrad, nmissed);

    if(valuestackFlag)
    {
      if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+3)*sizeof(char *)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return; }
      for(i=0; i<3; i++)
      {
        if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
        { printf("\n\nERROR: realloc failure, valuestack\n\n"); return; }
      }
      sprintf(valuestack[valuestack_ptr++],"%e", dist_min );
      sprintf(valuestack[valuestack_ptr++],"%e", dist_max );
      sprintf(valuestack[valuestack_ptr++],"%e", sum_dist/ndist );
      printf(" in inverse order written to stack\n");
    }
    return;
  }

 skipTriProj:;

  /* dependent higher entities need new label-positions, achived with orient */
  /* create a set with the dependent higher entities to be oriented  */

  /* add all points to special set _ORI  */
  if(set[setNr].anz_p>0)
  { 
    j=pre_seta( specialset->ori, "i", 0 );
    for (i=0; i<set[setNr].anz_p; i++)
    {
      p=set[setNr].pnt[i];
      seta( j, "p", p );
    }
    completeSet( specialset->ori, "up");
    orientSet( specialset->ori );
    delSet(specialset->ori);
  }

  /* recalculate the shapes (from command-interpreter "REP") */
  for (i=0; i<anzGeo->l; i++) repLine(i);

  /* fix midside-node positions */
  fixMidsideNodes(set[setNr].name, "");

  /* reposition the internal drawing nodes */
  if(anz->e>0)
  {
    adjustDrawNodes(0);
    //makeSurfaces(); instead:
    getFaceNormalen( face, node, anz );
    getElemNormalen( e_enqire, node, anz->e );
    updateDispLists();
  }
  free_proj(nx, nrad, nmissed);
}


void  generateSetIndexes()
{
  int i, index=1;
  for (i=0; i<anz->sets; i++)
  {
    if( set[i].name != (char *)NULL )
    {
      if (!set[i].type)
      {
        set[i].index=index++;
      }
    }
  }
}


/*------------------------------------------------------------------*/
/* setinhalt schreiben                                              */
/*------------------------------------------------------------------*/

int prnt(char *record)
{
  int   setNr, nr, p, l, nc,nl, args, nmin, emin;
  int i,j,k,n;
  int length, bias_fbd, foundSubString;
  char name[MAX_LINE_LENGTH], typ[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH];
  char **dat;
  char *str=NULL, *token=NULL, *saveptr=NULL;
  double r1[3], r2[3];

  typ[0]=param[0]=name[0]=0;
  length=sscanf(record,"%s%s%s",typ,name,param);
  if((length==2)&&(compareStrings(name, "*")>0)) length--;

  /* generate the set-indexes */
  generateSetIndexes();

  if ((compare( typ, "col", 3) == 3) ||(compare( typ, "COL", 3) == 3))
  {
    for(i=0; i<entitycols; i++)
    {
      printf("%d %s %f %f %f\n", i+1, entitycol[i].name, entitycol[i].r, entitycol[i].g, entitycol[i].b  );
    }
    return(1);
  }
  if ((compare( typ, "usr", 3) == 3) ||(compare( typ, "USR", 3) == 3))
  {
    if (!anz->u) printf(" no user headers defined\n");
    /* list all existing parameters */
    for (i=0; i<anz->u; i++)
    {
        if( anz->uheader[i] != (char *)NULL )
        {
          printf ("%s\n", &anz->uheader[i][6]);
          strcpy(parameter[0], &anz->uheader[i][6]);
          write2stack(1, parameter);
        }
    }
    return(1);
  }
  if ((compare( typ, "par", 3) == 3) ||(compare( typ, "PAR", 3) == 3))
  {
    if (!lcase[cur_lc].npheader) printf(" no parameters for active dataset:%d defined\n", cur_lc+1);
    if (length==1)
    {
      /* list all existing parameters */
      for (i=0; i<lcase[cur_lc].npheader; i++)
      {
        if( lcase[cur_lc].pheader[i] != (char *)NULL )
        {
          printf ("%s\n", &lcase[cur_lc].pheader[i][6]);
          strcpy(parameter[0], &lcase[cur_lc].pheader[i][6]);
          write2stack(1, parameter);
        }
      }
      return(1);
    }
    else
    {
      for (i=0; i<lcase[cur_lc].npheader; i++)
      {
        if( lcase[cur_lc].pheader[i] != (char *)NULL )
        {
          if(compare(&lcase[cur_lc].pheader[i][6],name,strlen(name))==strlen(name))
	  {
            printf ("%s\n", &lcase[cur_lc].pheader[i][6]);
            strcpy(parameter[0], &lcase[cur_lc].pheader[i][6]);
            write2stack(1, parameter);
	  }
        }
      } 
      return(1);
    }
  }

  if ((compare( typ, "amp", 3) == 3) ||(compare( typ, "AMP", 3) == 3))
  {
    if (!anz->amps) printf(" no amplitudes defined\n");
    if (length==1)
    {
      /* list all existing amplitudes */
      for (i=0; i<anz->amps; i++)
      {
        if( amplitude[i].name != (char *)NULL )
        {
          printf ("%s xy-pairs:%d\n", amplitude[i].name, amplitude[i].n);
        }
      }
      return(1);
    }
    else
    {
      setNr=getAmplitudeNr(name,1);
      if (setNr<0)
      {
        /* amp was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->amps; setNr++)
	  {
            for(i=0; i<length; i++)
	    {
              if(strstr(amplitude[setNr].name, dat[i]) !=NULL)
	      {
                if( amplitude[setNr].name != (char *)NULL )
                {
                  printf ("%s xy-pairs:%d\n", amplitude[setNr].name, amplitude[setNr].n);
                }
	      }
	    }
	  }
          return(0);
	}
        printf (" prnt: amplitude:'%s' does not exist\n", name);
        return (-1);
      }
    }
    printf ("i      x:            y:\n");
    for (i=0; i<amplitude[setNr].n; i++)
    {
      printf (" %8d %12e %12e\n", i+1, amplitude[setNr].x[i],amplitude[setNr].y[i]);
    }
  }

  if ((compare( typ, "mat", 3) == 3) ||(compare( typ, "MAT", 3) == 3))
  {
    if (!anz->mats) printf(" no materials defined\n");
    if (length==1)
    {
      /* list all existing materials */
      for (i=0; i<anz->mats; i++)
      {
        if( material[i].name != (char *)NULL )
        {
          printf ("%s elastic:%d expansion:%d conductivity:%d specific heat:%d plastic:%d\n", material[i].name, material[i].nela, material[i].nexp, material[i].ncon, material[i].nsph, material[i].npl);
        }
      }
      return(1);
    }
    else
    {
      setNr=getMatNr(name,1);
      if (setNr<0)
      {
        /* amp was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->mats; setNr++)
	  {
            for(i=0; i<length; i++)
	    {
              if(strstr(material[setNr].name, dat[i]) !=NULL)
	      {
                if( material[setNr].name != (char *)NULL )
                {
                  printf ("%s elastic:%d expansion:%d conductivity:%d specific heat:%d \n", material[setNr].name, material[setNr].nela, material[setNr].nexp, material[setNr].ncon, material[setNr].nsph);
                }
	      }
	    }
	  }
          return(0);
	}
        printf (" prnt: material:'%s' does not exist\n", name);
        return(-1);
      }
    }
    printf ("density:%e\n", material[setNr].rho);
    printf ("\ni      elastic:      nue:      temperature:\n");
    for (i=0; i<material[setNr].nela; i++)
    {
      printf (" %8d %12e %12e %12e\n", i+1, material[setNr].ela[i],material[setNr].nue[i],material[setNr].tela[i]);
    }
    printf ("\ni      expansion:      temperature:\n");
    for (i=0; i<material[setNr].nexp; i++)
    {
      printf (" %8d %12e %12e\n", i+1, material[setNr].exp[i],material[setNr].texp[i]);
    }
    printf ("\ni      conductivity:      temperature:\n");
    for (i=0; i<material[setNr].ncon; i++)
    {
      printf (" %8d %12e %12e\n", i+1, material[setNr].con[i],material[setNr].tcon[i]);
    }
    printf ("\ni      sp. heat:      temperature:\n");
    for (i=0; i<material[setNr].nsph; i++)
    {
      printf (" %8d %12e %12e\n", i+1, material[setNr].sph[i],material[setNr].tsph[i]);
    }
    printf ("\ni      stress:      pl.strain:    temperature:\n");
    for (i=0; i<material[setNr].npl; i++)
    {
      printf (" %8d %12e %12e %12e\n", i+1, material[setNr].spl[i],material[setNr].epl[i],material[setNr].tpl[i]);
    }
  }

  if ((compare( typ, "st", 2) == 2) ||(compare( typ, "ST", 2) == 2))
  {
    if ((compare( name, "size", 2) == 2) ||(compare( name, "SIZE", 2) == 2))
    {
      printf ("%d\n",valuestack_ptr);
      sprintf(parameter[0], "%d", valuestack_ptr);
      write2stack(1, parameter);
    }
    else
    {
      i=valuestack_ptr-1;
      if(i<0) printf(" stack is empty\n");
      else
      {
        j=0;
        while(0<=i)
        {
          printf ("%d %s\n",++j,valuestack[i]); i--;
        }
      }
    }
  }
  else if ((compare( typ, "info", 2) == 2) ||(compare( typ, "INFO", 2) == 2))
  {
    if(anz->nmin == MAX_INTEGER) nmin=0; else nmin=anz->nmin; 
    if(anz->emin == MAX_INTEGER) emin=0; else emin=anz->emin;

    printf ("%s %s  n:%d e:%d f:%d ed:%d t:%d se:%d mat:%d amp:%d ds:%d nmax:%d nmin:%d emax:%d emin:%d nnext:%d enext:%d mesh-threads:%d\n", datin, anz->model, set[0].anz_n, anz->e, anz->f, anz->g, anz->t, anz->sets, anz->mats, anz->amps, anz->l, anz->orignmax, nmin, anz->emax, emin, anz->nnext, anz->enext, anz->threads);
    printf ("p:%d l:%d c:%d s:%d b:%d sh:%d L:%d S:%d psets:%d ",anzGeo->p, anzGeo->l, anzGeo->c, anzGeo->s, anzGeo->b, anzGeo->sh, anzGeo->nurl, anzGeo->nurs, anzGeo->psets);
    if(meshp.tetmesher==0) printf("netgen\n");
    else printf("tetgen\n");
    sprintf(parameter[0],"%s",datin);
    sprintf(parameter[1],"%s",anz->model);
    sprintf(parameter[2],"%d",set[0].anz_n);
    sprintf(parameter[3],"%d",anz->e);
    sprintf(parameter[4],"%d",anz->f);
    sprintf(parameter[5],"%d",anz->g);
    sprintf(parameter[6],"%d",anz->t);
    sprintf(parameter[7],"%d",anz->sets);
    sprintf(parameter[8],"%d",anz->mats);
    sprintf(parameter[9],"%d",anz->amps);
    sprintf(parameter[10],"%d",anz->l);
    sprintf(parameter[11],"%d",anz->orignmax);
    sprintf(parameter[12],"%d",nmin);
    sprintf(parameter[13],"%d",anz->emax);
    sprintf(parameter[14],"%d",emin);
    sprintf(parameter[15],"%d",anz->nnext);
    sprintf(parameter[16],"%d",anz->enext);
    sprintf(parameter[17],"%d",anz->threads);
    sprintf(parameter[18],"%d",anzGeo->p);
    sprintf(parameter[19],"%d",anzGeo->l);
    sprintf(parameter[20],"%d",anzGeo->c);
    sprintf(parameter[21],"%d",anzGeo->s);
    sprintf(parameter[22],"%d",anzGeo->b);
    sprintf(parameter[23],"%d",anzGeo->sh);
    sprintf(parameter[24],"%d",anzGeo->nurl);
    sprintf(parameter[25],"%d",anzGeo->nurs);
    sprintf(parameter[26],"%d",anzGeo->psets);
    if(meshp.tetmesher==0) sprintf(parameter[27],"netgen\n");
    else sprintf(parameter[27],"tetgen\n");
    write2stack( 28, parameter);
  }
  else if ((compare( typ, "ve", 2) == 2) ||(compare( typ, "VE", 2) == 2))
  {
    // list the sets of all visible entities
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if(pset[j].type[0]=='e')
      {
        if(!i++) printf(" visible elements:\n");
	if(pset[j].type[1]=='v')
	  printf("  set[%d]: %s color: values\n",set[pset[j].nr].index, set[pset[j].nr].name);
        else
	  printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if(pset[j].type[0]=='f')
      {
        if(!i++) printf(" visible faces:\n");
	if(pset[j].type[1]=='v')
	  printf("  set[%d]: %s color: values\n",set[pset[j].nr].index, set[pset[j].nr].name);
        else
          printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='n')
      {
        if(!i++) printf(" visible nodes:\n");
	if(pset[j].type[1]=='v')
	  printf("  set[%d]: %s color: values\n",set[pset[j].nr].index, set[pset[j].nr].name);
        else
          printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='p')
      {
        if(!i++) printf(" visible points:\n");
        printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='l')
      {
        if(!i++) printf(" visible lines:\n");
        printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='s')
      {
        if(!i++) printf(" visible surfaces:\n");
        printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='b')
      {
        if(!i++) printf(" visible bodies:\n");
        printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='L')
      {
        if(!i++) printf(" visible nurl:\n");
        printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
    i=0;
    for (j=0; j<anzGeo->psets; j++ )
    {
      if (pset[j].type[0]=='S')
      {
        if(!i++) printf(" visible nurs:\n");
        printf("  set[%d]: %s color: %s\n",set[pset[j].nr].index, set[pset[j].nr].name, entitycol[pset[j].col].name);
      }
    }
  }
  else if ((compare( typ, "se", 2) == 2) ||(compare( typ, "SE", 2) == 2))
  {
    if (!anz->sets)
    {
      printf("ERROR: no sets defined\n");
      strcpy(parameter[0], "ERROR: no sets defined");
      write2stack(1, parameter);
    }
    if (length==1)
    {
      /* list all existing sets */
      for (i=0; i<anz->sets; i++)
      {
        if( set[i].name != (char *)NULL )
        {
          if ((!set[i].type)&&(set[i].name[0]!=':'))
          {
            printf ("%-5d %s stat:%c n:%d e:%d f:%d p:%d l:%d c:%d s:%d b:%d L:%d S:%d se:%d sh:%d v:%d\n", set[i].index, set[i].name, set[i].flag, set[i].anz_n, set[i].anz_e, set[i].anz_f, set[i].anz_p, set[i].anz_l, set[i].anz_c, set[i].anz_s, set[i].anz_b, set[i].anz_nurl, set[i].anz_nurs, set[i].anz_se, set[i].anz_sh, set[i].anz_v);
            sprintf(parameter[0],"%d",set[i].index);
            sprintf(parameter[1],"%s",set[i].name);
            sprintf(parameter[2],"%c",set[i].flag);
            sprintf(parameter[3],"%d",set[i].anz_n);
            sprintf(parameter[4],"%d",set[i].anz_e);
            sprintf(parameter[5],"%d",set[i].anz_f);
            sprintf(parameter[6],"%d",set[i].anz_p);
            sprintf(parameter[7],"%d",set[i].anz_l);
            sprintf(parameter[8],"%d",set[i].anz_c);
            sprintf(parameter[9],"%d",set[i].anz_s);
            sprintf(parameter[10],"%d",set[i].anz_b);
            sprintf(parameter[11],"%d",set[i].anz_nurl);
            sprintf(parameter[12],"%d",set[i].anz_nurs);
            sprintf(parameter[13],"%d",set[i].anz_se);
            sprintf(parameter[14],"%d",set[i].anz_sh);
            sprintf(parameter[15],"%d",set[i].anz_v);
            write2stack( 16, parameter);
          }
        }
      }
      return(1);
    }
    else
    {
      operateAlias( name, "se" );
      setNr=getSetNr(name);
      if((setNr>=0)&&(compare(param,"range",3)==3))
      {
        if((set[setNr].anz_n>2)&&(set[setNr].anz_e>2))
	{
          param[0]=0;
          /* check for holes */
          for(i=0; i<set[setNr].anz_n-1; i++)
            if((set[setNr].node[i]+1)!=(set[setNr].node[i+1])) break;
	  if(i<set[setNr].anz_n-1) printf("Warning set %s has holes in the node numbering starting between node %d and %d\n", set[setNr].name, set[setNr].node[i], set[setNr].node[i+1]);
          for(i=0; i<set[setNr].anz_e-1; i++)
            if((set[setNr].elem[i]+1)!=(set[setNr].elem[i+1])) break;
	  if(i<set[setNr].anz_e-1) printf("Warning set %s has holes in the elem numbering starting between elem %d and %d\n", set[setNr].name, set[setNr].elem[i], set[setNr].elem[i+1]);			       					       

          printf ("%-5d %s nr %d %d er %d %d\n", set[setNr].index, set[setNr].name, set[setNr].node[0], set[setNr].node[set[setNr].anz_n-1], set[setNr].elem[0], set[setNr].elem[set[setNr].anz_e-1]);
	}
        return(1);
      }

      if (setNr<0)
      {
        /* set was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
	  {
            foundSubString=0;
            for(i=0; i<length; i++)
	    {
              if(strstr(set[setNr].name, dat[i]) !=NULL)
	      {
	        foundSubString++;
	        /* check if the first or the last char is no '*' then the dat[] must be at start or end */
	        if(i==0) { if(name[0]!='*')  { if(name[0]!=set[setNr].name[0])  foundSubString--; }  }
         	if(i==length-1) { if(name[strlen(name)-1]!='*') { if(name[strlen(name)-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
	      }
	    }
            if(foundSubString==length)
	    {
	      i=setNr;
              if (!set[i].type)
              {
		j++;
                printf ("%-5d %s stat:%c n:%d e:%d f:%d p:%d l:%d c:%d s:%d b:%d L:%d S:%d se:%d sh:%d v:%d", set[i].index, set[i].name, set[i].flag, set[i].anz_n, set[i].anz_e, set[i].anz_f, set[i].anz_p, set[i].anz_l, set[i].anz_c, set[i].anz_s, set[i].anz_b, set[i].anz_nurl, set[i].anz_nurs, set[i].anz_se, set[i].anz_sh, set[i].anz_v);
                sprintf(parameter[0],"%d", set[i].index	    );
                sprintf(parameter[1],"%s", set[i].name	    );
                sprintf(parameter[2],"%c", set[i].flag	    );
                sprintf(parameter[3],"%d", set[i].anz_n	    );
                sprintf(parameter[4],"%d", set[i].anz_e	    );
                sprintf(parameter[5],"%d", set[i].anz_f	    );
                sprintf(parameter[6],"%d", set[i].anz_p	    );
                sprintf(parameter[7],"%d", set[i].anz_l	    );
                sprintf(parameter[8],"%d", set[i].anz_c	    );
                sprintf(parameter[9],"%d", set[i].anz_s	    );
                sprintf(parameter[10],"%d", set[i].anz_b	    );
                sprintf(parameter[11],"%d", set[i].anz_nurl   );
                sprintf(parameter[12],"%d", set[i].anz_nurs   );
                sprintf(parameter[13],"%d", set[i].anz_se    );
                sprintf(parameter[14],"%d", set[i].anz_sh    );
                sprintf(parameter[15],"%d", set[i].anz_v      );
                if(compare(param,"range",3)==3)
		{
                  printf (" nr %d %d er %d %d", set[setNr].node[0], set[setNr].node[set[setNr].anz_n-1], set[setNr].elem[0], set[setNr].elem[set[setNr].anz_e-1]);
                  sprintf(parameter[16],"%d", set[i].node[0]   );
                  sprintf(parameter[17],"%d", set[i].node[set[setNr].anz_n-1]  );
                  sprintf(parameter[18],"%d", set[i].elem[0]  );
                  sprintf(parameter[19],"%d", set[i].elem[set[setNr].anz_e-1]   );
                  printf ("\n");
                  write2stack(20, parameter);
		}
                else
	        {
                  printf ("\n");
                  write2stack(16, parameter);
		}
              }
	    }
	  }
          if(j!=0)
	  {
            /* free dat */
            for(i=0; i<length; i++) free(dat[i]);
            free(dat);
	    return(1);
	  }
	}
        /* free dat */
        for(i=0; i<length; i++) free(dat[i]);
        free(dat);

        printf ("ERROR: set %s does not exist\n", name);
        sprintf(parameter[0], "ERROR: set %s does not exist", name);
        write2stack(1, parameter);
        return (-1);
      }
    }
    if (set[setNr].type) return(0);

    if(anz->l)
    {
      printf (" node:    value:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%6e",lcase[cur_lc].dat[cur_entity][set[setNr].node[i]]);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[4],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3],parameter[4]);
        write2stack(5, parameter);
      }
    }
    else
    {
      printf (" node:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3]);
        write2stack(4, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_e; i++)
    {
      nr=set[setNr].elem[i];
      printf (" elem:%d type:%d n:", nr, e_enqire[nr].type);
      if     (e_enqire[nr].type==1) k=8;
      else if(e_enqire[nr].type==2) k=6;
      else if(e_enqire[nr].type==3) k=4;
      else if(e_enqire[nr].type==4) k=20;
      else if(e_enqire[nr].type==5) k=15;
      else if(e_enqire[nr].type==6) k=10;
      else if(e_enqire[nr].type==7) k=3;
      else if(e_enqire[nr].type==8) k=6;
      else if(e_enqire[nr].type==9) k=4;
      else if(e_enqire[nr].type==10) k=8;
      else if(e_enqire[nr].type==11) k=2;
      else if(e_enqire[nr].type==12) k=3;
      else k=0;
      for (n=0; n<k; n++) printf("%d ",e_enqire[nr].nod[n]);
      printf("\n"); 
      sprintf(parameter[0],"%d", nr);
      sprintf(parameter[1],"%d", e_enqire[nr].type);
      for (n=0; n<k; n++) sprintf(parameter[n+2],"%d", e_enqire[nr].nod[n]);
      write2stack(k+2, parameter);
    }
    for (i=0; i<set[setNr].anz_f; i++)
    {
      nr=set[setNr].face[i];
      printf (" face:%d elem:%d side:%d nodes:",  nr, face[nr].elem_nr, face[nr].nr+1);
      if(face[nr].type==7) k=3;
      else if(face[nr].type==8) k=6;
      else if(face[nr].type==9) k=4;
      else if(face[nr].type==10) k=8;
      else if(face[nr].type==11) k=2;
      else if(face[nr].type==12) k=3;
      else k=0;
      for (n=0; n<k; n++) printf("%d ",face[nr].nod[n]);
      printf("\n"); 
      sprintf(parameter[0],"%d", nr);
      sprintf(parameter[1],"%d", face[nr].elem_nr);
      sprintf(parameter[2],"%d", face[nr].nr+1);
      for (n=0; n<k; n++) sprintf(parameter[n+3],"%d", face[nr].nod[n]);
      write2stack(k+3, parameter);
    }
    for (i=0; i<set[setNr].anz_p; i++)
    {
      p=set[setNr].pnt[i];
      if( point[p].name != (char *)NULL ) 
        printf (" pnt:%s %lf %lf %lf\n", point[p].name,
        (point[p].px* scale->w+scale->x),
        (point[p].py* scale->w+scale->y),
        (point[p].pz* scale->w+scale->z) );
      sprintf(parameter[0],"%s", point[p].name);
      sprintf(parameter[1],"%lf", point[p].px* scale->w+scale->x);
      sprintf(parameter[2],"%lf", point[p].py* scale->w+scale->y);
      sprintf(parameter[3],"%lf", point[p].pz* scale->w+scale->z);
      write2stack(4, parameter);
    }
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l=set[setNr].line[i];
      if( line[l].name != (char *)NULL ) 
      {
        bias_fbd=getBias_fbd(l, line);

        if( line[l].typ=='a' )  /* arc-line */
        {
          printf (" line:%s typ:a p1:%s p2:%s pc:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d lock:%d\n"
          , line[l].name, point[line[l].p1].name,
          point[line[l].p2].name, point[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr, line[l].elock);
          sprintf(parameter[4],"%s",point[line[l].trk].name);
        }
        else if( line[l].typ=='s' )  /* seq-line */
        {
          printf (" line:%s typ:s p1:%s p2:%s set:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d lock:%d\n"
          , line[l].name, point[line[l].p1].name,
          point[line[l].p2].name, set[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr, line[l].elock);
          sprintf(parameter[4],"%s",set[line[l].trk].name);
        }
        else  /* its a straight line   */
        {
          printf (" line:%s typ:l p1:%s p2:%s trk:- div:%d bias:%d bias_el:%lf etyp:%d attr:%d lock:%d\n"
          , line[l].name, point[line[l].p1].name,
          point[line[l].p2].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr, line[l].elock);
          sprintf(parameter[4],"-");
        }
        sprintf(parameter[0],"%s",line[l].name);
        sprintf(parameter[1],"%c",line[l].typ);
        sprintf(parameter[2],"%s",point[line[l].p1].name);
        sprintf(parameter[3],"%s",point[line[l].p2].name);
        sprintf(parameter[5],"%d",line[l].div);
        sprintf(parameter[6],"%d",bias_fbd);
        sprintf(parameter[7],"%f",line[l].bias);
        sprintf(parameter[8],"%d",line[l].etyp);
        sprintf(parameter[9],"%d",line[l].eattr);
        write2stack(10, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_c; i++)
    {
      p=set[setNr].lcmb[i];
      if( lcmb[p].name != (char *)NULL ) 
      {
        printf (" lcmb:%s uses %d lines:", lcmb[p].name, lcmb[p].nl);
        for (j=0; j<lcmb[p].nl; j++)
          printf (" %1c %s", lcmb[p].o[j], line[lcmb[p].l[j]].name );
        printf (" \n");
        sprintf(parameter[0],"%s",lcmb[p].name);
        sprintf(parameter[1],"%d",lcmb[p].nl);
        j=2;
        for (n=0; n<lcmb[p].nl; n++) { sprintf(parameter[j++],"%1c", lcmb[p].o[n]); sprintf(parameter[j++],"%s", line[lcmb[p].l[n]].name); }
        write2stack(j, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_s; i++)
    {
      p=set[setNr].surf[i];
      if( surf[p].name != (char *)NULL ) 
      {
	  // printf("sh:%d\n",surf[p].sh);
        if (surf[p].sh<=-1) printf (" surf:%s %1c BLEND ", surf[p].name, surf[p].ori );
        else if (surf[p].sh>-1) printf (" surf:%s %1c %s ", surf[p].name, surf[p].ori, shape[surf[p].sh].name );
        sprintf(parameter[0],"%s", surf[p].name);
        sprintf(parameter[1],"%c", surf[p].ori);
        if (surf[p].sh<=-1) sprintf(parameter[2],"BLEND");
        else  sprintf(parameter[2],"%s", shape[surf[p].sh].name);
        for (j=0; j<surf[p].nl; j++)
        {
          nl=surf[p].l[j];
          if (surf[p].typ[j]=='l')
          {
            printf (" %1c %s", surf[p].o[j], line[nl].name );
          }
          else
          {
            printf (" %1c %s", surf[p].o[j], lcmb[nl].name );
            for (k=0; k<lcmb[nl].nl; k++ )
            {
              nc=lcmb[nl].l[k];
              printf (" (%c) (%s)", lcmb[nl].o[k], line[nc].name );
            }
          }
        }
        if( surf[p].eparm != (char *)NULL ) printf (" etyp:%d attr:%d lock:%d mshp:%s\n", surf[p].etyp, surf[p].eattr, surf[p].elock, surf[p].eparm);
        else printf (" etyp:%d attr:%d lock:%d\n", surf[p].etyp, surf[p].eattr, surf[p].elock);
      }
      write2stack(3, parameter);
    }
    for (i=0; i<set[setNr].anz_b; i++)
    {
      p=set[setNr].body[i];
      if( body[p].name != (char *)NULL ) 
      {
        printf (" body:%s %1c", body[p].name, body[p].ori );
        sprintf(parameter[0],"%s", body[p].name);
        sprintf(parameter[1],"%c", body[p].ori);
        for (j=0; j<body[p].ns; j++)
          printf (" %1c %s", body[p].o[j], surf[body[p].s[j]].name );
        if( body[p].eparm != (char *)NULL ) printf (" etyp:%d attr:%d lock:%d mshp:%s\n", body[p].etyp, body[p].eattr, body[p].elock, body[p].eparm);
        else printf (" etyp:%d attr:%d lock:%d\n", body[p].etyp, body[p].eattr, body[p].elock);
      }
      write2stack(2, parameter);
    }
    for (i=0; i<set[setNr].anz_nurl; i++)
    {
      nr=set[setNr].nurl[i];
      if( nurbl[nr].name != (char *)NULL ) 
      {
        printf (" nurl:%s", nurbl[nr].name );
        printf (" exp:%d pnts:%d nods:%d End:%d\n", nurbl[nr].u_exp, nurbl[nr].u_npnt, nurbl[nr].u_nknt, nurbl[nr].endFlag);
        /*
        for (k=0; k<nurbl[nr].u_nknt; k++) printf("u-node[%d] = %lf\n", k+1, nurbl[nr].uknt[k]);
        for (k=0; k<nurbl[nr].u_npnt; k++)
        {
          printf("cpnt[%d]:%s x=%lf y=%lf z=%lf w=%lf\n", k+1, point[nurbl[nr].ctlpnt[k]].name, point[nurbl[nr].ctlpnt[k]].px* scale->w+scale->x,
          point[nurbl[nr].ctlpnt[k]].py* scale->w+scale->y, point[nurbl[nr].ctlpnt[k]].pz* scale->w+scale->z, nurbl[nr].weight[k]);
        }
	*/
      }
    }
    for (i=0; i<set[setNr].anz_nurs; i++)
    {
      nr=set[setNr].nurs[i];
     if( nurbs[nr].name != (char *)NULL ) 
     {
      printf (" nurs:%s", nurbs[nr].name );
      printf (" u_exp:%d v_exp:%d u_npnt:%d v_npnt:%d u_nnod:%d v_nnod:%d End:%d\n", nurbs[nr].u_exp, nurbs[nr].v_exp,
        nurbs[nr].u_npnt, nurbs[nr].v_npnt, nurbs[nr].u_nknt, nurbs[nr].v_nknt, nurbs[nr].endFlag);
      /*
      for (k=0; k<nurbs[nr].u_nknt; k++) printf("u-node[%d] = %lf\n", k+1, nurbs[nr].uknt[k]);
      for (k=0; k<nurbs[nr].v_nknt; k++) printf("v-node[%d] = %lf\n", k+1, nurbs[nr].vknt[k]);
      for (k=0; k<nurbs[nr].u_npnt; k++)
      {
        for (j=0; j<nurbs[nr].v_npnt; j++)
        {
          printf("cpnt[%d][%d]:%s x=%lf y=%lf z=%lf w=%lf \n", k+1, j+1, point[nurbs[nr].ctlpnt[k][j]].name, point[nurbs[nr].ctlpnt[k][j]].px* scale->w+scale->x,
          point[nurbs[nr].ctlpnt[k][j]].py* scale->w+scale->y, point[nurbs[nr].ctlpnt[k][j]].pz* scale->w+scale->z, nurbs[nr].weight[k][j]);
        }
      }
      */
     }
    }
    for (i=0; i<set[setNr].anz_se; i++)
    {
      p=set[setNr].set[i];
      if( set[p].name != (char *)NULL ) 
      {
        printf (" set:%s ", set[p].name );
        printf (" \n");
      }
    }
    for (i=0; i<set[setNr].anz_sh; i++)
    {
      p=set[setNr].shp[i];
      if( shape[p].name != (char *)NULL ) 
      {
        if(shape[p].type==0)
          printf (" shape:%s PLN p1:%s p2:%s p3:%s\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, point[shape[p].p[2]].name );
        if(shape[p].type==1)
        {
          v_result( &point[shape[p].p[0]].px, &point[shape[p].p[2]].px, r1  );
          printf (" shape:%s CYL p1:%s p2:%s r:%f (p3:%s)\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, v_betrag(r1)*scale->w, point[shape[p].p[2]].name );
	}
        if(shape[p].type==2)
        {
          v_result( &point[shape[p].p[0]].px, &point[shape[p].p[2]].px, r1  );
          v_result( &point[shape[p].p[1]].px, &point[shape[p].p[3]].px, r2  );
          printf (" shape:%s CONE p1:%s p2:%s r1:%f (p3:%s) r2:%f (p4:%s)\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, v_betrag(r1)*scale->w, point[shape[p].p[2]].name, v_betrag(r2)*scale->w, point[shape[p].p[3]].name );
	}
        if(shape[p].type==3)
        {
          v_result( &point[shape[p].p[0]].px, &point[shape[p].p[1]].px, r1  );
          printf (" shape:%s SPH p1:%s r:%f (p2:%s)\n", shape[p].name, point[shape[p].p[0]].name, v_betrag(r1)*scale->w, point[shape[p].p[1]].name );
	}
        if(shape[p].type==4)
          printf (" shape:%s NURS %s\n", shape[p].name, nurbs[shape[p].p[0]].name);
        if(shape[p].type==5)
        {
          v_result( &point[shape[p].p[0]].px, &point[shape[p].p[2]].px, r1  );
          v_result( &point[shape[p].p[2]].px, &point[shape[p].p[3]].px, r2  );
          printf (" shape:%s TOR p1:%s p2:%s r1:%f (p3:%s) r2:%f (p4:%s)\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, v_betrag(r1)*scale->w, point[shape[p].p[2]].name, v_betrag(r2)*scale->w, point[shape[p].p[3]].name );
	}
      }
    }
    for (i=0; i<set[setNr].anz_v; i++)
    {
      p=set[setNr].valu[i];
      if( value[p].name != (char *)NULL ) 
      {
        printf (" value:%s string:%s\n", value[p].name, value[p].string);
      }
    }
    printf (" %d nodes, %d elements, %d faces, %d Points, %d Lines, %d Lcmb, %d Surfs, %d Bodys, %d Nurl, %d Nurs, %d sets, %d shapes, %d values stored\n", set[setNr].anz_n, set[setNr].anz_e, set[setNr].anz_f, set[setNr].anz_p, set[setNr].anz_l, set[setNr].anz_c, set[setNr].anz_s, set[setNr].anz_b, set[setNr].anz_nurl, set[setNr].anz_nurs, set[setNr].anz_se, set[setNr].anz_sh, set[setNr].anz_v );
    sprintf(parameter[0],"%d",set[setNr].anz_n);
    sprintf(parameter[1],"%d",set[setNr].anz_e);
    sprintf(parameter[2],"%d",set[setNr].anz_f);
    sprintf(parameter[3],"%d",set[setNr].anz_p);
    sprintf(parameter[4],"%d",set[setNr].anz_l);
    sprintf(parameter[5],"%d",set[setNr].anz_c);
    sprintf(parameter[6],"%d",set[setNr].anz_s);
    sprintf(parameter[7],"%d",set[setNr].anz_b);
    sprintf(parameter[8],"%d",set[setNr].anz_nurl);
    sprintf(parameter[9],"%d",set[setNr].anz_nurs);
    sprintf(parameter[10],"%d",set[setNr].anz_se);
    sprintf(parameter[11],"%d",set[setNr].anz_sh);
    sprintf(parameter[12],"%d",set[setNr].anz_v);
    write2stack( 13, parameter);
  }

  else if ((compare( typ, "sh", 2) == 2) ||(compare( typ, "SH", 2) == 2))
  {
    l=getShapeNr(name);
    if (l<0)
    {
      printf (" prnt: shape:%s does not exist\n", name);
      return (-1);
    }
    if( shape[l].name != (char *)NULL ) 
    {
      if(shape[l].type==0)
        printf (" shape:%s PLN p1:%s p2:%s p3:%s\n", shape[l].name, point[shape[l].p[0]].name, point[shape[l].p[1]].name, point[shape[l].p[2]].name );
      if(shape[l].type==1)
      {
        v_result( &point[shape[l].p[0]].px, &point[shape[l].p[2]].px, r1  );
        printf (" shape:%s CYL p1:%s p2:%s r:%f (p3:%s)\n", shape[l].name, point[shape[l].p[0]].name, point[shape[l].p[1]].name, v_betrag(r1)*scale->w, point[shape[l].p[2]].name );
	}
      if(shape[l].type==2)
      {
        v_result( &point[shape[l].p[0]].px, &point[shape[l].p[2]].px, r1  );
        v_result( &point[shape[l].p[1]].px, &point[shape[l].p[3]].px, r2  );
        printf (" shape:%s CONE p1:%s p2:%s r1:%f (p3:%s) r2:%f (p4:%s)\n", shape[l].name, point[shape[l].p[0]].name, point[shape[l].p[1]].name, v_betrag(r1)*scale->w, point[shape[l].p[2]].name, v_betrag(r2)*scale->w, point[shape[l].p[3]].name );
	}
      if(shape[l].type==3)
      {
        v_result( &point[shape[l].p[0]].px, &point[shape[l].p[1]].px, r1  );
        printf (" shape:%s SPH p1:%s r:%f (p2:%s)\n", shape[l].name, point[shape[l].p[0]].name, v_betrag(r1)*scale->w, point[shape[l].p[1]].name );
	}
      if(shape[l].type==4)
        printf (" shape:%s NURS %s\n", shape[l].name, nurbs[shape[l].p[0]].name);
      if(shape[l].type==5)
      {
        v_result( &point[shape[l].p[0]].px, &point[shape[l].p[2]].px, r1  );
        v_result( &point[shape[l].p[2]].px, &point[shape[l].p[3]].px, r2  );
        printf (" shape:%s TOR p1:%s p2:%s r1:%f (p3:%s) r2:%f (p4:%s)\n", shape[l].name, point[shape[l].p[0]].name, point[shape[l].p[1]].name, v_betrag(r1)*scale->w, point[shape[l].p[2]].name, v_betrag(r2)*scale->w, point[shape[l].p[3]].name );
      }
    }
  }
  else if ((compare( typ, "sq", 2) == 2) ||(compare( typ, "SQ", 2) == 2))
  {
    if (length==1)
    {
      /* list all existing sequential sets */
      l=0;
      for (i=0; i<anz->sets; i++)
      {
        if( set[i].name != (char *)NULL )
        {
          if (set[i].type)
	  {
            l++;
            printf (" %s type:SEQ stat:%c n:%d p:%d\n", set[i].name, set[i].flag, set[i].anz_n, set[i].anz_p);
	  }
        }
      }
      if (!l) printf(" no sequences defined\n");
      return(1);
    }
    else 
    {
      setNr=getSetNr(name);
      if (setNr<0)
      {
        /* set was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
	  {
            foundSubString=0;
            for(i=0; i<length; i++)
            {
              if(strstr(set[setNr].name, dat[i]) !=NULL)
	      {
	        foundSubString++;
	        /* check if the first or the last char is no '*' then the dat[] must be at start or end */
	        if(i==0) { if(name[0]!='*')  { if(name[0]!=set[setNr].name[0])  foundSubString--; }  }
         	if(i==length-1) { if(name[strlen(name)-1]!='*') { if(name[strlen(name)-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
	      }
	    }
            if(foundSubString==length)
	    {
	      i=setNr;
              if (set[i].type)
              {
                printf ("%s type:SEQ stat:%c n:%d p:%d\n", set[i].name, set[i].flag, set[i].anz_n, set[i].anz_p);
		j++;
              }
	    }
	  }
          if(j!=0) return(1);
	}
        printf (" prnt: seq:%s does not exist\n", name);
        return(-1);
      }
    }
    if (!set[setNr].type) return(0);

    if(anz->l)
    {
      printf (" node:    value:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%6e",lcase[cur_lc].dat[cur_entity][set[setNr].node[i]]);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[4],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3],parameter[4]);
        write2stack(5, parameter);
      }
    }
    else
    {
      printf (" node:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3]);
        write2stack(4, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_p; i++)
    {
      p=set[setNr].pnt[i];
      if( point[p].name != (char *)NULL ) 
        printf (" pnt:%s x:%lf y:%lf z:%lf\n", point[p].name,
        (point[p].px* scale->w+scale->x),
        (point[p].py* scale->w+scale->y),
        (point[p].pz* scale->w+scale->z) );
    }
  }

  /* check entities */
  else if ((typ[0]=='n')||(typ[0]=='N'))
  {
    nr=atoi(name);
    if((nr<1)||(nr>anz->nmax)) { printf (" prnt: node:'%s' does not exist\n", name);return (-1); }
    if(anz->l)
    {
      printf (" node:%d v:%lf x:%lf y:%lf z:%lf\n", nr, lcase[cur_lc].dat[cur_entity][nr],
        (node[nr].nx* scale->w+scale->x),
        (node[nr].ny* scale->w+scale->y),
        (node[nr].nz* scale->w+scale->z) );
      sprintf(parameter[0],"%d", nr);
      sprintf(parameter[1],"%lf",lcase[cur_lc].dat[cur_entity][nr]);
      sprintf(parameter[2],"%lf",node[nr].nx * scale->w+scale->x);
      sprintf(parameter[3],"%lf",node[nr].ny * scale->w+scale->y);
      sprintf(parameter[4],"%lf",node[nr].nz * scale->w+scale->z);
      write2stack(5, parameter);
    }
    else
    {
      printf (" node:%d xyz: %lf %lf %lf\n", nr,
        (node[nr].nx* scale->w+scale->x),
        (node[nr].ny* scale->w+scale->y),
        (node[nr].nz* scale->w+scale->z) );
      sprintf(parameter[0],"%d", nr);
      sprintf(parameter[1],"%lf",node[nr].nx * scale->w+scale->x);
      sprintf(parameter[2],"%lf",node[nr].ny * scale->w+scale->y);
      sprintf(parameter[3],"%lf",node[nr].nz * scale->w+scale->z);
      write2stack(4, parameter);
    }
  }
  else if ((typ[0]=='e')||(typ[0]=='E'))
  {
    if (typ[1]=='q')
    {
      operateAlias( name, "se" );
      setNr=getSetNr(name);
      if (setNr<0)
      {
        printf (" prnt: set:%s does not exist\n", name);
        return(-1);
      }
      /* show bad elements */
      i=calcBadElements(set[setNr].name);
      if (i>0)
      {
        printf("found %d bad elements in set:%s (stored in set:%s)\n", i, set[setNr].name, specialset->njby);
	setNr=getSetNr(specialset->njby);
        for (i=0; i<set[setNr].anz_e; i++)
        {
          nr=set[setNr].elem[i];
          printf (" elem:%d type:%d n:", nr, e_enqire[nr].type);
          if     (e_enqire[nr].type==1) k=8;
          else if(e_enqire[nr].type==2) k=6;
          else if(e_enqire[nr].type==3) k=4;
          else if(e_enqire[nr].type==4) k=20;
          else if(e_enqire[nr].type==5) k=15;
          else if(e_enqire[nr].type==6) k=10;
          else if(e_enqire[nr].type==7) k=3;
          else if(e_enqire[nr].type==8) k=6;
          else if(e_enqire[nr].type==9) k=4;
          else if(e_enqire[nr].type==10) k=8;
          else if(e_enqire[nr].type==11) k=2;
          else if(e_enqire[nr].type==12) k=3;
          else k=0;
          for (n=0; n<k; n++) printf("%d ",e_enqire[nr].nod[n]);
          printf("\n"); 
        }
      }
      else { printf("found no bad element in set:%s\n", name); delSet(specialset->njby); }
      return(1);
    }
    nr=atoi(name);
    printf (" elem:%d ",  nr);
    if     (e_enqire[nr].type==1) k=8;
    else if(e_enqire[nr].type==2) k=6;
    else if(e_enqire[nr].type==3) k=4;
    else if(e_enqire[nr].type==4) k=20;
    else if(e_enqire[nr].type==5) k=15;
    else if(e_enqire[nr].type==6) k=10;
    else if(e_enqire[nr].type==7) k=3;
    else if(e_enqire[nr].type==8) k=6;
    else if(e_enqire[nr].type==9) k=4;
    else if(e_enqire[nr].type==10) k=8;
    else if(e_enqire[nr].type==11) k=2;
    else if(e_enqire[nr].type==12) k=3;
    else k=0;
    for (n=0; n<k; n++) printf("%d ",e_enqire[nr].nod[n]);
    printf("\n"); 
  }
  else if ((typ[0]=='f')||(typ[0]=='F'))
  {
    nr=atoi(name);
    printf (" face:%d elem:%d side:%d nodes:",  nr, face[nr].elem_nr, face[nr].nr+1);
      if(face[nr].type==7) k=3;
      else if(face[nr].type==8) k=6;
      else if(face[nr].type==9) k=4;
      else if(face[nr].type==10) k=8;
      else if(face[nr].type==11) k=2;
      else if(face[nr].type==12) k=3;
      else k=0;
      for (n=0; n<k; n++) printf("%d ",face[nr].nod[n]);
      printf("\n"); 
  }
  else if ((typ[0]=='p')||(typ[0]=='P'))
  {
    operateAlias( name, "p" );
    p=getPntNr(name);
    if (p<0)
    {
      printf (" prnt: point:%s does not exist\n", name);
      return (-1);
    }
    if( point[p].name != (char *)NULL )
    {
      printf (" pnt:%s x:%lf y:%lf z:%lf\n", point[p].name,
      (point[p].px* scale->w+scale->x),
      (point[p].py* scale->w+scale->y),
      (point[p].pz* scale->w+scale->z) );
      sprintf(parameter[0],"%s", point[p].name);
      sprintf(parameter[1],"%lf", point[p].px* scale->w+scale->x);
      sprintf(parameter[2],"%lf", point[p].py* scale->w+scale->y);
      sprintf(parameter[3],"%lf", point[p].pz* scale->w+scale->z);
      write2stack(4, parameter);
    }
  }
  else if ((typ[0]=='l')||(typ[0]=='L'))
  {
    operateAlias( name, "l" );
    l=getLineNr(name);
    if (l<0)
    {
      printf (" prnt: line:%s does not exist\n", name);
      return (-1);
    }
    if( line[l].name != (char *)NULL ) 
    {
      bias_fbd=getBias_fbd(l, line);

      if( line[l].typ=='a' )  /* arc-line */
      {
        printf (" line:%s p1:%s p2:%s pc:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d lock:%d\n"
        , line[l].name, point[line[l].p1].name,
        point[line[l].p2].name, point[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr, line[l].elock);
      }
      else if( line[l].typ=='s' )  /* seq-line */
      {
        printf (" line:%s p1:%s p2:%s set:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d lock:%d\n"
        , line[l].name, point[line[l].p1].name,
        point[line[l].p2].name, set[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr, line[l].elock);
      }
      else  /* its a straight line   */
      {
        printf (" line:%s p1:%s p2:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d lock:%d\n"
        , line[l].name, point[line[l].p1].name,
        point[line[l].p2].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr, line[l].elock);
      }
    }
  }
  else if ((typ[0]=='c')||(typ[0]=='C'))
  {
    operateAlias( name, "c" );
    l=getLcmbNr(name);
    if (l<0)
    {
      printf (" prnt: lcmb:%s does not exist\n", name);
      return (-1);
    }  
    if( lcmb[l].name != (char *)NULL ) 
    {
      printf (" lcmb:%s uses %d lines:", lcmb[l].name, lcmb[l].nl);
      for (j=0; j<lcmb[l].nl; j++)
        printf (" %1c %s", lcmb[l].o[j], line[lcmb[l].l[j]].name );
      printf (" \n");
    }
  }
  else if (typ[0]=='s')
  {
    operateAlias( name, "s" );
    p=getSurfNr(name);
    if (p<0)
    {
      printf (" prnt: surf:%s does not exist\n", name);
      return (-1);
    }  
    if( surf[p].name != (char *)NULL ) 
    {
      if (surf[p].sh<=-1) printf (" surf:%s %1c BLEND ", surf[p].name, surf[p].ori );
      else if (surf[p].sh>-1) printf (" surf:%s %1c %s ", surf[p].name, surf[p].ori, shape[surf[p].sh].name );
      for (j=0; j<surf[p].nl; j++)
      {
        nl=surf[p].l[j];
        if (surf[p].typ[j]=='l')
        {
          printf (" %1c %s", surf[p].o[j], line[nl].name );
        }
        else
        {
          printf (" %1c %s", surf[p].o[j], lcmb[nl].name );
          for (k=0; k<lcmb[nl].nl; k++ )
          {
            nc=lcmb[nl].l[k];
            printf (" (%c) (%s)", lcmb[nl].o[k], line[nc].name );
          }
        }
      }
      if( surf[p].eparm != (char *)NULL ) printf (" etyp:%d attr:%d lock:%d mshp:%s\n", surf[p].etyp, surf[p].eattr, surf[p].elock, surf[p].eparm);
      else printf (" etyp:%d attr:%d lock:%d\n", surf[p].etyp, surf[p].eattr, surf[p].elock);
    }
  }
  else if (typ[0]=='S')
  {
    operateAlias( name, "S" );
    nr=getNursNr(name);
    if (nr<0)
    {
      printf (" prnt: Nurbs:%s does not exist\n", name);
      return (-1);
    }  
    if( nurbs[nr].name != (char *)NULL ) 
    {
      printf (" nurs:%s", nurbs[nr].name );
      printf (" u_exp:%d v_exp:%d u_npnt:%d v_npnt:%d u_nnod:%d v_nnod:%d End:%d\n", nurbs[nr].u_exp, nurbs[nr].v_exp,
        nurbs[nr].u_npnt, nurbs[nr].v_npnt, nurbs[nr].u_nknt, nurbs[nr].v_nknt, nurbs[nr].endFlag);
      
      for (k=0; k<nurbs[nr].u_nknt; k++) printf("u-node[%d] = %lf\n", k+1, nurbs[nr].uknt[k]);
      for (k=0; k<nurbs[nr].v_nknt; k++) printf("v-node[%d] = %lf\n", k+1, nurbs[nr].vknt[k]);
      for (k=0; k<nurbs[nr].u_npnt; k++)
      {
        for (j=0; j<nurbs[nr].v_npnt; j++)
        {
          printf("cpnt[%d][%d]:%s x=%lf y=%lf z=%lf w=%lf \n", k+1, j+1, point[nurbs[nr].ctlpnt[k][j]].name, point[nurbs[nr].ctlpnt[k][j]].px* scale->w+scale->x,
          point[nurbs[nr].ctlpnt[k][j]].py* scale->w+scale->y, point[nurbs[nr].ctlpnt[k][j]].pz* scale->w+scale->z, nurbs[nr].weight[k][j]);
        }
      }
      
    }
  }
  else if ((typ[0]=='b')||(typ[0]=='B'))
  {
    operateAlias( name, "b" );
    p=getBodyNr(name);
    if (p<0)
    {
      printf (" prnt: body:%s does not exist\n", name);
      return (-1);
    }  
    if( body[p].name != (char *)NULL ) 
    {
      printf (" body:%s %1c", body[p].name, body[p].ori );
      for (j=0; j<body[p].ns; j++)
        printf (" %1c %s", body[p].o[j], surf[body[p].s[j]].name );
      if( body[p].eparm != (char *)NULL ) printf (" etyp:%d attr:%d lock:%d mshp:%s\n", body[p].etyp, body[p].eattr, body[p].elock, body[p].eparm);
      else printf (" etyp:%d attr:%d lock:%d\n", body[p].etyp, body[p].eattr, body[p].elock);
    }
  }
  else if ((typ[0]=='v')||(typ[0]=='V'))
  {
    if (length==1)
    {
      /* list all existing values */
      for (i=0; i<anz->v; i++)
      {
        if( value[i].name != (char *)NULL )
        {
          printf (" value %s stores %s\n", value[i].name, value[i].string );
        }
      }
      return(1);
    }
    p=getValuNr(name);
    if (p<0)
    {
      printf (" prnt: value:%s does not exist\n", name);
      return (-1);
    }  
    if( value[p].name != (char *)NULL ) 
    {
      printf (" value %s stores %s\n", value[p].name, value[p].string );

      /* split the value[p].string in separate strings at ' ' occurences and write them to the stack */
      strcpy(param,value[p].string);
      for(args=0, str=param; ; args++, str=NULL)
      {
        token = strtok_r(str," ", &saveptr);
        if(token == NULL) break;
        if(args == 20) break;
        strcpy(parameter[args],token);
      }
      write2stack(args, parameter);
    }
  }
  return(1);
}

/*------------------------------------------------------------------*/
/* set und setinhalt loeschen                                       */
/*------------------------------------------------------------------*/

int zap(char *record)
{
  int i,j,k,n,s,b;
  int anz_s=0,anz_b=0;
  int setNr;
  int addDispFlagLocal=0;
  int *surfbuf, *bodybuf;
  char *cori;
  int *edge;
  char setname[MAX_LINE_LENGTH];
  Gbod onebody;

  sword( record, setname );
  operateAlias( setname, "se" );
  setNr=getSetNr(setname);

  if (setNr<0) return (-1);

  /* search dependent lines, redefine the line to straight to keep it */
  for ( i=0; i<set[setNr].anz_p; i++)
  {
    for (j=0; j<anzGeo->l; j++)
    {
      if(( set[setNr].pnt[i] == line[j].trk )&&( line[j].typ == 'a' ))
      {
        line[j].typ = ' ';
      }
    }
  }

  // shapes are special in a way that they can be deleted wo destroying related
  // higher entities (surfaces), surfaces are just converted to BLEND
  if (set[setNr].anz_sh>0)
  {
    for ( i=0; i<set[setNr].anz_sh; i++)
    {
      j=set[setNr].shp[i];
      for(k=0; k<shape[set[setNr].shp[i]].ns; k++)
      {
        s=shape[set[setNr].shp[i]].s[k];
	if(surf[s].pgn!=NULL)
	{
          surf[s].sh=-1; free(surf[s].pgn); surf[s].pgn=NULL; surf[s].npgn=0;
          repSurf(s,1);
	}
        else
        {
          surf[s].sh=-1; free(surf[s].pgn); surf[s].pgn=NULL; surf[s].npgn=0;
	}
        printf (" interior of surf %s changed to BLEND\n",surf[s].name);
      }
    }

    delShape( set[setNr].anz_sh, set[setNr].shp );
    printf(" shapes deleted\n");
  }

  // delete the initially included higher entities later
  if (set[setNr].anz_b>0)
  {
    anz_b=set[setNr].anz_b;
    if ( (bodybuf = (int *)malloc( (set[setNr].anz_b+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
    for(i=0; i<set[setNr].anz_b; i++ )
      bodybuf[i]=set[setNr].body[i];
  }

  if (set[setNr].anz_s>0)
  {
    anz_s=set[setNr].anz_s;
    if ( (surfbuf = (int *)malloc( (set[setNr].anz_s+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
    for(i=0; i<set[setNr].anz_s; i++ )
      surfbuf[i]=set[setNr].surf[i];
  }

  /* vor dem loeschen muessen noch die abhaengigen direkt uebergeordneten Groessen dazugeladen werden  */
  printf(" complete set\n");
  completeSet( setname, "up");
  printf(" set completed\n");
  
  if (anz_b>0)
  {
    delBody( anz_b, bodybuf );
    printf(" bodies deleted\n");
    free(bodybuf);
  }
  if (anz_s>0)
  {
    delSurf( anz_s, surfbuf );
    printf(" surfs deleted\n");
    free(surfbuf);
  }

  if (set[setNr].anz_e>0)
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    delElem( set[setNr].anz_e, set[setNr].elem );
    printf(" elements deleted\n");
  }
  if (set[setNr].anz_n>0)
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    delNod( set[setNr].anz_n, set[setNr].node );
    printf(" nodes deleted\n");
  }
  if(addDispFlagLocal==2) { addDispToCoordinates(node); }
  
  if (set[setNr].anz_c>0)
  {
    delLcmb( set[setNr].anz_c, set[setNr].lcmb );
    printf(" lcmbs deleted\n");
  }
  if (set[setNr].anz_l>0)
  {
    delLine( set[setNr].anz_l, set[setNr].line );
    printf(" lines deleted\n");
  }
  if (set[setNr].anz_p>0)
  {
    delPnt( set[setNr].anz_p, set[setNr].pnt );
    printf(" points deleted\n");
  }
  if (set[setNr].anz_v>0)
  {
    delVal( set[setNr].anz_v, set[setNr].valu );
    printf(" values deleted\n");
  }

  /* search dependent surfaces, redefine if necessary */
  for(i=0; i<set[setNr].anz_s; i++)
  {
    j=set[setNr].surf[i];
    s=surf[j].nl;
    
    k=0;
    for (n=0; n<surf[j].nl; n++)
    {
      if((surf[j].typ[n]=='l')&&(line[surf[j].l[n]].name!=NULL))
      {
        surf[j].typ[k]=surf[j].typ[n];
        surf[j].l[k]=surf[j].l[n];
        surf[j].o[k]=surf[j].o[n];
        k++;
      }
    }
    surf[j].nl=k;
  
    if ( orientSurf( j ) <0 ); // errMsg ("ERROR: orientSurf:%s failed\n", surf[j].name);
    else
    {
      printf(" Surf:%s redefined\n", surf[j].name);
      if(s>surf[j].nl) { setr(setNr, "s", j); i--; }
    }

  }
  if (set[setNr].anz_s>0)
  {
    delSurf( set[setNr].anz_s, set[setNr].surf );
    printf(" dependent surfs deleted\n");

    /* remove to the remaining surfs related shapes from the del-set */
    for ( i=0; i<anzGeo->s; i++)
    {
      if(( surf[i].name != (char *)NULL )&&( surf[i].sh>-1))
      {
	//printf("surf:%s uses shape %s\n",surf[i].name,shape[surf[i].sh].name);
	j=set[setNr].anz_sh;
	setr( setNr, "sh", surf[i].sh);
	if(j>set[setNr].anz_sh) printf(" shape:%s still needed by surf:%s and saved\n",shape[surf[i].sh].name, surf[i].name);
      }
    }
  }
  if (set[setNr].anz_nurs>0)
  {
    /* remove to the remaining shape related nurbs from the del-set */
    for ( i=0; i<anzGeo->sh; i++)
    {
      //printf("%d shpe:%s nurbs:%s\n",i,shape[i].name,  nurbs[shape[i].p[0]].name);
      //for(j=0; j<shape[j].ns; j++) printf("%d surf:%s\n",j,surf[shape[j].s[j]].name);
      if(( shape[i].name != (char *)NULL )&&( shape[i].type==4))
      {
        j=set[setNr].anz_nurs;
	setr( setNr, "S", shape[i].p[0]);
        if(j>set[setNr].anz_nurs) printf(" nurbs:%s still needed and saved\n",nurbs[shape[i].p[0]].name);
      }
    }

    delNurs( set[setNr].anz_nurs, set[setNr].nurs );
    printf(" nurs and dependent nurs deleted\n");
  }

  if (set[setNr].anz_b>0)
  {
    //delBody( set[setNr].anz_b, set[setNr].body);
    // or:
    //check each body if the surfaces are still there, then do not delete
    for(i=0; i<set[setNr].anz_b; i++)
    {
      b= set[setNr].body[i];

      // delete all bodies which do not use tets
      //if((body[b].etyp!=3)&&(body[b].etyp!=6)) { delBody( 1, &b ); i--; continue; }
      
      if ( (cori = (char *)malloc( (body[b].ns+1) * sizeof(char))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      if ( (edge = (int *)malloc( (body[b].ns+1) * sizeof(int))) == NULL )
        printf("\n\n ERROR: malloc failure\n\n" );
      s=0;
      for (j=0; j<body[b].ns; j++)
      {
        if(surf[body[b].s[j]].name!=NULL)
	{
	  cori[s]=body[b].o[j];
	  edge[s]=body[b].s[j];
	  s++;
	}
      }
      if(s>2)
      {
        onebody.etyp=body[b].etyp;
        onebody.eattr=body[b].eattr;
        onebody.elock=body[b].elock;
        onebody.eparm=body[b].eparm;
	if (gbod_i( body[b].name, 0, s, cori, edge )<0) delBody( 1, &b );
        body[b].etyp=onebody.etyp;
        body[b].eattr=onebody.eattr;
        body[b].elock=onebody.elock;
        body[b].eparm=onebody.eparm;
      }
      else
      {
        delBody( 1, &b );
	i--;
      }
      free(cori);
      free(edge);
    }
    
    printf(" dependent bodies deleted\n");
  }

  /* delete the set itself */
  delSet(setname);

  return(1);
}



void mata( int *elemMat, int mat, int setNr )
{
  int  i, j;
  int           *ielem=NULL;

  /* mark the elements of the set for easy identification */
  if( (ielem=(int *)malloc((anz->emax+1)*sizeof(int) ) )==NULL) 
  { printf(" ERROR: malloc failure\n"); return; }
  if (setNr==-1)
    for (i=0; i<=anz->emax; i++) ielem[i]=1;
  else
  {
    for (i=0; i<=anz->emax; i++) ielem[i]=0;
    for (i=0; i<set[setNr].anz_e; i++) ielem[set[setNr].elem[i]]=1;
  }

  /* exists this mat? */
  for (i=1; i<=elemMat[0]; i++) if (elemMat[i]==mat) break;
  if(i>elemMat[0])
  {
    elemMat[0] = i;
    elemMat[i] = mat;
  }
  
  for (j=0; j<anz->e; j++)
  {
    if (ielem[e_enqire[j].nr])
      e_enqire[e_enqire[j].nr].mat= elemMat[i];
  }
  free(ielem); ielem=NULL;
}


char getLetter(int i)
{
  if(i>9) return(i-10+65);
  else return(i+48);
}


int getFamName( int n, char *c )
{
  int i, j;
  double f;

  j=n/46656;
  n=n%46656;

  /* change decimal (0->46655) into a string of 3 char (000 -> ZZZ) */
  f=n/1296.;
  i=f+1e-10;
  c[0]=getLetter(i);
  f=(f-i)*36.;
  i=f+1e-10;
  c[1]=getLetter(i);
  i=(f-i)*36.+1e-10;
  c[2]=getLetter(i);
  c[3]='\0';
  return(j);
}


int getNewName( char *name, char *type )
{
  int i,nr,nr2=0;
  static int p, l, c, s, b, L, S, se, sh;
  char string[MAX_LINE_LENGTH];

  /* not implemented yet 
  if(fillSpaceFlag) p=l=c=s=b=S=se=0;
  */

    do
    {
      if (type[0]=='p')
      {
        p++;
        i=getFamName(p, string);
        if(lchar->p+i>90)  sprintf(name, "%c%-d", lchar->p, p );
        else sprintf(name, "%c%s", lchar->p+i, string );
        nr=getPntNr(name);
      }
      else if (compare( type, "se", 2) == 2 )
      {
        se++;
        i=getFamName(se, string);
        if(lchar->se+i>90)  sprintf(name, "%c%-d", lchar->se,se);
        else sprintf(name, "%c%s", lchar->se+i, string );
        nr=getSetNr(name);
      }
      else if (compare( type, "sh", 2) == 2 )
      {
        sh++;
        i=getFamName(sh, string);
        if(lchar->sh+i>90)  sprintf(name, "%c%-d", lchar->sh,sh);
        else sprintf(name, "%c%s", lchar->sh+i, string );
        nr=getShapeNr(name);
      }
      else if (type[0]=='l')
      {
        l++;
        i=getFamName(l, string);
        if(lchar->l+i>90)  sprintf(name, "%c%-d", lchar->l, l );
        else sprintf(name, "%c%s", lchar->l+i, string );
        nr=getLineNr(name);
        if(nr<0) nr2=getLcmbNr(name); if(nr2>0) nr=0; /* check if a lcmb has this name, not use it */
      }
      else if (type[0]=='c')
      {
        c++;
        i=getFamName(c, string);
        if(lchar->c+i>90)  sprintf(name, "%c%-d", lchar->c, c );
        else sprintf(name, "%c%s", lchar->c+i, string );
        nr=getLcmbNr(name);
        if(nr<0) nr2=getLineNr(name); if(nr2>0) nr=0; /* check if a line has this name, not use it */
      }
      else if (type[0]=='s')
      {
        s++;
        i=getFamName(s, string);
        if(lchar->s+i>90)  sprintf(name, "%c%-d", lchar->s, s );
        else sprintf(name, "%c%s", lchar->s+i, string );
        nr=getSurfNr(name);
      }
      else if (type[0]=='b')
      {
        b++;
        i=getFamName(b, string);
        if(lchar->b+i>90)  sprintf(name, "%c%-d", lchar->b, b );
        else sprintf(name, "%c%s", lchar->b+i, string );
        nr=getBodyNr(name);
      }
      else if (type[0]=='L')
      {
        L++;
        i=getFamName(L, string);
        if(lchar->L+i>90)  sprintf(name, "%c%-d", lchar->L, L );
        else sprintf(name, "%c%s", lchar->L+i, string );
        nr=getNurlNr(name);
      }
      else if (type[0]=='S')
      {
        S++;
        i=getFamName(S, string);
        if(lchar->S+i>90)  sprintf(name, "%c%-d", lchar->S, S );
        else sprintf(name, "%c%s", lchar->S+i, string );
        nr=getNursNr(name);
      }
      else
        return(-1);
    }while(nr>-1);   /* new entity */

  return(1);
}



double calcGTOL(int setNr)
{
  int l,i;
  double max[3], min[3], gtol;

  /* calculate the geometric tolerance based on all line-end-points and nodes */
  for(i=0; i<3; i++)
  {
    max[i]=-MAX_FLOAT;
    min[i]= MAX_FLOAT;
  }
  for(l=0; l<set[setNr].anz_l; l++)
  {
    i=set[setNr].line[l];
    if(line[i].name!=(char *)NULL)
    {
      if(point[line[i].p1].px > max[0]) max[0]=point[line[i].p1].px;
      if(point[line[i].p2].px > max[0]) max[0]=point[line[i].p2].px;
      if(point[line[i].p1].py > max[1]) max[1]=point[line[i].p1].py;
      if(point[line[i].p2].py > max[1]) max[1]=point[line[i].p2].py;
      if(point[line[i].p1].pz > max[2]) max[2]=point[line[i].p1].pz;
      if(point[line[i].p2].pz > max[2]) max[2]=point[line[i].p2].pz;
  
      if(point[line[i].p1].px < min[0]) min[0]=point[line[i].p1].px;
      if(point[line[i].p2].px < min[0]) min[0]=point[line[i].p2].px;
      if(point[line[i].p1].py < min[1]) min[1]=point[line[i].p1].py;
      if(point[line[i].p2].py < min[1]) min[1]=point[line[i].p2].py;
      if(point[line[i].p1].pz < min[2]) min[2]=point[line[i].p1].pz;
      if(point[line[i].p2].pz < min[2]) min[2]=point[line[i].p2].pz;
    }
  }
  for(l=0; l<set[setNr].anz_n; l++)
  {
    i=set[setNr].node[l];
    if(node[i].pflag!=1)
    {
      if(node[i].nx > max[0]) max[0]=node[i].nx;
      if(node[i].nx > max[0]) max[0]=node[i].nx;
      if(node[i].ny > max[1]) max[1]=node[i].ny;
      if(node[i].ny > max[1]) max[1]=node[i].ny;
      if(node[i].nz > max[2]) max[2]=node[i].nz;
      if(node[i].nz > max[2]) max[2]=node[i].nz;

      if(node[i].nx < min[0]) min[0]=node[i].nx;
      if(node[i].nx < min[0]) min[0]=node[i].nx;
      if(node[i].ny < min[1]) min[1]=node[i].ny;
      if(node[i].ny < min[1]) min[1]=node[i].ny;
      if(node[i].nz < min[2]) min[2]=node[i].nz;
      if(node[i].nz < min[2]) min[2]=node[i].nz;
    }
  }
  gtol=GTOL;
  for(i=0; i<3; i++)
  {
    max[i]-=min[i];
    if(max[i]>gtol) gtol=max[i];
  }
  gtol=GTOL*gtol*scale->w;

  return(gtol);
}



int readlist(char *datin, char *type)
{
  FILE *handle;

  int length, i, j, n, dotpos, column, setNr;
  char string[MAX_LINE_LENGTH], face[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH];
  int nr;

  handle = fopen (datin, "r");
  if (handle==NULL)
  {
    printf (" ERROR in readlist: The input file \"%s\" could not be opened.\n\n", datin);
    return(-1);
  }
  else  printf ("\n%s opened",datin);

  length = 1;
  printf ("\n reading file\n");

  i=n=0;
  dotpos = strlen (datin);
  for (j=0; j<dotpos; j++) if (datin[j]=='/') n=j+1;
  for (j=n; j<dotpos; j++)
  {
    if (datin[j]=='.')
    {
       break;
    }
    else datin[i]=datin[j];
    i++;
  }
  datin[i]= '\0';

  /* initialize the set */
  if( (setNr=pre_seta( datin, "is", 0)) <0 ) return(-1);
 
  /* get the column */
  if(strlen(type)>1) column=atoi(&type[1])-1;
  else column=0;

  if((type[0]=='E')||(type[0]=='e'))
  {
    for (i=0; i<column; i++) sprintf(&format[i*4], "%%*s ");
    sprintf(&format[i*4], "%%d, %%s");
    while (length > -1)
    {
      length = frecord( handle, string);
      if( string[length]== (char)EOF)  break;
      n=sscanf(string, format, &nr, face );
      printf("rec:%d e:%d face:%s\n",n,nr,face);
      if(n)
      {
        printf (" seta %s e %d\n", datin, nr );
        seta( setNr, "e", nr );
        if(n==2) /* element face is provided */
	{
          if(e_enqire[nr].type==1)
          {
	    if(face[1]=='1')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
	    }
	    else if(face[1]=='2')
            {
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
	    }
	    else if(face[1]=='3')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
	    }
	    else if(face[1]=='4')
            {
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
	    }
	    else if(face[1]=='5')
            {
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
	    }
	    else if(face[1]=='6')
            {
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
	    }
            else printf(" side:%s not supported\n", face);
	  }
          else if(e_enqire[nr].type==6)
          {
	    if(face[1]=='1')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
	    }
	    else if(face[1]=='2')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[8] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
	    }
	    else if(face[1]=='3')
            {
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[9] );
              seta( setNr, "n", e_enqire[nr].nod[8] );
	    }
	    else if(face[1]=='4')
            {
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
              seta( setNr, "n", e_enqire[nr].nod[9] );
	    }
            else printf(" side:%s not supported\n", face);
	  }
          else printf(" element type:%d not supported\n", e_enqire[nr].type);
	}
      }
    }
  }
  else if((type[0]=='N')||(type[0]=='n'))
  {
    for (i=0; i<column; i++) sprintf(&format[i*4], "%%*s ");
    sprintf(&format[i*4], "%%d");
    while (length > -1)
    {
      length = frecord( handle, string);
      if( string[length]== (char)EOF)  break;
      n=sscanf((const char *)string, (const char *)format, &nr );
      printf("rec:%d n:%d\n",n,nr);
      if(n)
      {
        printf (" seta %s n %d\n", datin, nr );
        seta( setNr, "n", nr );
      }
    }
  }
  else printf (" ERROR: type %s not known\n", type );
  fclose(handle);
  printf ("\n A sequence of name:%s was created. Please use 'prnt sq' for listings \n\n", set[setNr].name );
  return(1);
}


void pre_norm(char *name)
{
  int i,j, setNr, setcopy;
  int *sum_n=NULL;
  Nodes *norm=NULL;

  setNr=getSetNr(name);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", name);
    goto errorPreNorm;
  }

  /* determine the normal based on all connected faces */
  delSet("+norm");
  setcopy=pre_seta( "+norm", "i", 0);
  if (setcopy<0)
  {
    errMsg (" ERROR: set:+norm could not be created\n" );
    goto errorPreNorm;
  }
  for (i=0; i<set[setNr].anz_n; i++)
  {
    seta( setcopy, "n", set[setNr].node[i] );
  }
  completeSet("+norm", "up") ;
  completeSet("+norm", "do") ;
  getNodeNormalen(&sum_n, &norm, setcopy, anz, face);
  delSet("+norm");

  for(i=0; i<set[setNr].anz_n; i++)
  {
    printf("node:%d norm:%f %f %f\n",set[setNr].node[i], norm[set[setNr].node[i]].nx, norm[set[setNr].node[i]].ny ,norm[set[setNr].node[i]].nz);
    if(valuestackFlag)
    {
      if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+4)*sizeof(char *)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); goto errorPreNorm; }
      for(j=0; j<4; j++)
      {
        if ((valuestack[valuestack_ptr+j] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
        { printf("\n\nERROR: realloc failure, valuestack\n\n"); goto errorPreNorm; }
      }
      sprintf(valuestack[valuestack_ptr++],"%e", norm[set[setNr].node[i]].nz );
      sprintf(valuestack[valuestack_ptr++],"%e", norm[set[setNr].node[i]].ny );
      sprintf(valuestack[valuestack_ptr++],"%e", norm[set[setNr].node[i]].nx );
      sprintf(valuestack[valuestack_ptr++],"%d", set[setNr].node[i] );
      if(valuestackFlag) printf(" 4 values in inverse order written to stack\n");
    }
  }
 errorPreNorm:;
  free(sum_n);
  free(norm);
  return;
}

void pre_movie(char *string)
{
  int i,j,k, length;
  static int delay=10;
  static int loops=0;
  double val1=0, val2=0;
  char type[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH], movie[MAX_LINE_LENGTH];
  movie[0]=0;
  length=sscanf(string, "%s %lf %lf %s", type, &val1, &val2, movie );

  if (compareStrings(type, "delay")>0) { if(length==2) delay=100.*val1; else delay=10; }
  if (compareStrings(type, "loops")>0) { if(length==2) loops=val1; else loops=0; }
  if (compareStrings(type, "start")>0) { movieFlag=1; stopFlag=0; }
  if (compareStrings(type, "stop")>0)
  {
    printf("movie stopped, make movie with 'movie make <nr> <nr> [movie]'\n");
    printf("optionally define the delay-time between pictures with:'movie delay <sec>' before 'movi make'\n");
    printf("optionally reset the counter and delete the single frames with 'movi clean'\n");
    printf("you might use the program 'realplay' or 'firefox' to play the movie.gif file\n");
    movieFlag=0;
    sprintf( buffer, "rm -f  hcpy_0.tga %s", DEV_NULL2);
    system (buffer);
  }
  if (compareStrings(type, "frames")>0)
  {
    sprintf( buffer, "rm -f _*.gif %s", DEV_NULL2);
    system (buffer);
    gifNr=0;
    movieFlag=1;
    stopFlag=0;
    length=sscanf(string, "%*s %s %s", name, movie);
    if((length==1)&&(name[0]=='a'))
    { movieFrames=-1; movieCommandFile[0]=0; }
    else
    {
      movieFrames=(int)val1;
      length=sscanf(string, "%*s %*s %s", movieCommandFile);
      if(length<1) movieCommandFile[0]=0;
    }
    //printf("movieFrames:%d movieCommandFile:%s\n",movieCommandFile);
  }
  if (compareStrings(type, "clean")>0)
  {
    sprintf( buffer, "rm -f _*.gif %s", DEV_NULL2);
    system (buffer);
    gifNr=0;
  }
  if (compareStrings(type, "make")>0)
  {
    if(length==1) {  val2=gifNr, val1=1; length=3; }  
    else if(length==2) {  val2=val1, val1=1; length=3; }  

    if(length>2)
    {
      if(length==4)
      {
        printf("make movie from %s and pic:%d to %d, wait for ready\n", movie, (int)val1,(int)val2);
        sprintf( buffer, "cp %s movie.gif %s", movie, DEV_NULL2);
        system (buffer);
      }
      else
      {
        printf("make movie from pic:%d to %d, wait for ready\n", (int)val1,(int)val2);
        movie[0]=0;
      }
        
      /* generate movie from single gif files */
      j=k=0;
      for(i=(int)val1; i<=(int)val2; i++)
      { 
        sprintf(&name[j*10], "_%d.gif     ",i); j++;
        if(j==9)
        {
  	j=0;
	sprintf( buffer, "convert -loop %d -delay %d %s __%d.gif %s", loops, delay, name, k++,DEV_NULL2);
          system (buffer);
        }
      }
      if(j)
      {
        sprintf( buffer, "convert -loop %d -delay %d %s __%d.gif %s", loops, delay, name, k++,DEV_NULL2);
        system (buffer);
      }
  
      /* assemble all movies */
      j=0;
      for(i=0; i<k; i++)
      { 
        sprintf(&name[j*12], "__%d.gif     ",i); j++;
        if(j==9)
        {
  	  j=0;
          if (i==9) sprintf( buffer, "convert -loop %d -delay %d %s %s movie.gif %s", loops, delay, movie, name,DEV_NULL2);
          else sprintf( buffer, "convert -loop %d -delay %d movie.gif %s movie.gif %s", loops, delay, name,DEV_NULL2);
          system (buffer);
        }
      }
      if(j)
      {
        if (k<9) sprintf( buffer, "convert -loop %d -delay %d %s %s movie.gif %s", loops, delay, movie, name,DEV_NULL2);
        else sprintf( buffer, "convert -loop %d -delay %d movie.gif %s movie.gif %s", loops, delay, name,DEV_NULL2);
        system (buffer);
      }
      sprintf( buffer, "rm __*.gif %s", DEV_NULL2);
      system (buffer);
      printf("\nready\n");
      printf("\nyou might use the program 'realplay' or 'firefox' to play the movie.gif file\n\n");
    }
    else
    {
      printf(" ERROR: make movie with 'movie make <nr> <nr> [movie]'\n");
    }
  }
}



int calcCoefficientsTet(int nslav, int *emas, int n_closest_tets, Nodes *node, Tetraeder *tet, double cof[4], int extrapolflag)
{
  int i,e=0;
  int secondLoopFlag=0;
  double v12[3], v13[3], v14[3], vn[3], sum=0.;
  int closestElem=0;
  double closestDist=-MAX_FLOAT;

  /* ----------------------------------------- */
  //int j, el=1;
  //FILE *handle;
  /* ----------------------------------------- */

  for(e=0; e<n_closest_tets; e++)
  {
    secondLoop:;
    /* ----------------------------------------- */
    /*
    handle = fopen ( "int.inp", "a");
    fprintf(handle,"*NODE, NSET=Nall\n");
    j=nslav;
    fprintf(handle," %d,%f,%f,%f\n", j,node[j].nx,node[j].ny,node[j].nz  );
    for (i=0; i<4; i++)
    {
      j=tet[emas[e]].n[i];
      fprintf(handle," %d,%f,%f,%f\n", j,node[j].nx,node[j].ny,node[j].nz  );
    }
    fprintf(handle,"*ELEMENTS, TYPE=C3D4\n");
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[0], tet[emas[e]].n[1], tet[emas[e]].n[2], nslav);
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[1], tet[emas[e]].n[3], tet[emas[e]].n[2], nslav);
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[2], tet[emas[e]].n[3], tet[emas[e]].n[0], nslav);
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[3], tet[emas[e]].n[1], tet[emas[e]].n[0], nslav);
    fclose(handle);
    */
    /* ----------------------------------------- */

    /* calc volu, coef of opposite node is volu/tot_volu */
  
    /* volu = 1/6 * a x b * c */
    v_result( &node[tet[emas[e]].n[0]].nx, &node[tet[emas[e]].n[1]].nx, v12);   
    v_result( &node[tet[emas[e]].n[0]].nx, &node[tet[emas[e]].n[2]].nx, v13);   
    v_result( &node[tet[emas[e]].n[0]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[3]=v_sprod(vn,v14)/6./tet[emas[e]].v;
                                                    
    v_result( &node[tet[emas[e]].n[1]].nx, &node[tet[emas[e]].n[3]].nx, v12);   
    v_result( &node[tet[emas[e]].n[1]].nx, &node[tet[emas[e]].n[2]].nx, v13);   
    v_result( &node[tet[emas[e]].n[1]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[0]=v_sprod(vn,v14)/6./tet[emas[e]].v;
                                                    
    v_result( &node[tet[emas[e]].n[2]].nx, &node[tet[emas[e]].n[3]].nx, v12);   
    v_result( &node[tet[emas[e]].n[2]].nx, &node[tet[emas[e]].n[0]].nx, v13);   
    v_result( &node[tet[emas[e]].n[2]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[1]=v_sprod(vn,v14)/6./tet[emas[e]].v;
                                                    
    v_result( &node[tet[emas[e]].n[3]].nx, &node[tet[emas[e]].n[1]].nx, v12);   
    v_result( &node[tet[emas[e]].n[3]].nx, &node[tet[emas[e]].n[0]].nx, v13);   
    v_result( &node[tet[emas[e]].n[3]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[2]=v_sprod(vn,v14)/6./tet[emas[e]].v;

    if(printFlag) printf(" e:%d cof:%e %e %e %e\n", emas[e]+1, cof[0], cof[1], cof[2], cof[3]);

    if(secondLoopFlag) break;
  
    /* a negativ cof means the nslav is outside, try the next element */
    secondLoopFlag=1;
    sum=0.;
    for (i=0; i<4; i++)
    {
      if (cof[i]<0.)
      {
        sum+=cof[i];
        secondLoopFlag=0;
      }
    }
    if((sum<0.)&&(sum>closestDist)) { closestElem=e; closestDist=sum; }
    if(secondLoopFlag) break;
  }
  if(!secondLoopFlag)
  {
    /* take the element with the smallest negative cof */
    e=closestElem;
    secondLoopFlag=1;
    goto secondLoop;
  }
  if(e==n_closest_tets) return(-1); 

  /* a negativ cof means the nslav is outside, set the cof to 0 and scale the rest to a sum of 1 */
  sum=0.;
  for (i=0; i<4; i++)
  {
    if (cof[i]<=0.)
    {
      if(extrapolflag==-1) return(-1);  // strict!
      if((extrapolflag==0)&&(cof[i]<-1.e-2)) return(-1); // relaxed!
      cof[i]=0.;
    }
    else sum+=cof[i];
  }
  if(sum>0) for (i=0; i<4; i++) cof[i]/=sum;
  else return(-1);
  return(emas[e]);
}



int glob_map3d=0;
void *thread_interpol3d(void *vargp)
{
  int i,ii,j,e,n ;
  int set1, set2;
  int coarseFlag, extrapolflag;
  int ds=0, allds=0;
  int etet, tets=0, failedNodesSet;
  double  cof[4], dr, dx, dy, dz, tol;

  int near_node[N_CLOSEST_TETS];
  int     n_closest_nodes, n_closest_tets;
  int tetList[MAX_NR_OF_TETS_PER_ELEM];

  double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL;
  double *orig_ex=NULL, *orig_ey=NULL, *orig_ez=NULL, *sort_ex=NULL, *sort_ey=NULL, *sort_ez=NULL;
  int *sort_enx=NULL, *sort_eny=NULL, *sort_enz=NULL;

  Tetraeder *tet;
  Nodes     *nod;
  Elements  *elem;

  Tetraeder *tet2;
  Tetraeder *tetbuf;
  Elements  elem2;   

  typedef struct {
    Tetraeder *tet;
    Nodes     *nod;
    Elements  *elem;
    int vargi[9];
    double vargd[1];
    int *vargpi[6];
    double *vargpd[12];
  } Threadargs;
  Threadargs *param;

  param=(Threadargs *)vargp;

  tet=param->tet;
  nod=param->nod;
  elem=param->elem;
  tol=param->vargd[0];
  allds=param->vargi[2];
  set1=param->vargi[3];
  set2=param->vargi[4];
  extrapolflag=param->vargi[5];
  failedNodesSet=param->vargi[6];
  tets=param->vargi[7];
  ds=param->vargi[8];
  orig_x=param->vargpd[0];
  orig_y=param->vargpd[1];
  orig_z=param->vargpd[2];
  sort_x=param->vargpd[3];
  sort_y=param->vargpd[4];
  sort_z=param->vargpd[5];
  sort_nx=param->vargpi[0];
  sort_ny=param->vargpi[1];
  sort_nz=param->vargpi[2];
  orig_ex=param->vargpd[6];
  orig_ey=param->vargpd[7];
  orig_ez=param->vargpd[8];
  sort_ex=param->vargpd[9];
  sort_ey=param->vargpd[10];
  sort_ez=param->vargpd[11];
  sort_enx=param->vargpi[3];
  sort_eny=param->vargpi[4];
  sort_enz=param->vargpi[5];


  n_closest_nodes=1;

  while(1)
  {
    /* get the next node (i) */
    sem_wait(&sem_map3d);
    i=glob_map3d++;
    sem_post(&sem_map3d);
    if(i>=param->vargi[1]) break;
    //printf(" thread:%d node:%d tol:%e\n", param->vargi[0], set[set1].node[i], sqrt(tol));

    /* first, search for a close node */
    near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny,
           node[set[set1].node[i]].nz, set[set2].anz_n, &near_node[0], n_closest_nodes);
    //for (j=0; j<n_closest_nodes; j++) printf("node:%d near node:%d \n",set[set1].node[i], set[set2].node[near_node[j]]);
    dx= node[set[set2].node[near_node[0]]].nx - node[set[set1].node[i]].nx;
    dy= node[set[set2].node[near_node[0]]].ny - node[set[set1].node[i]].ny;
    dz= node[set[set2].node[near_node[0]]].nz - node[set[set1].node[i]].nz;      
    dr=dx*dx + dy*dy + dz*dz;      
    if(dr<tol )
    {
      if(printFlag) printf("node:%d, found equal node:%d with dr:%lf\n", set[set1].node[i], set[set2].node[near_node[0]], sqrt(dr));

      /* interpolate */
      if(allds)
      {
        /* all datasets should be interpolated */
        for(ds=0; ds<anz->l; ds++)
        {
          for(e=0; e<lcase[ds].ncomps; e++)
          {
            lcase[ds].dat[e][set[set1].node[i]]=lcase[ds].dat[e][ set[set2].node[near_node[0]] ];
            //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][ set[set1].node[i] ]);
          }
        }
      }
      else
      {
        for(e=0; e<lcase[ds].ncomps; e++)
        {
          lcase[ds].dat[e][set[set1].node[i]]=lcase[ds].dat[e][ set[set2].node[near_node[0]] ];
          //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][ set[set1].node[i] ]);
        }
      }
      continue;
    }

    if((int)N_CLOSEST_TETS/10<tets)  n_closest_tets= (int)N_CLOSEST_TETS/10; else n_closest_tets= tets;
    coarseFlag=1;
  secondLoop:;
    /* if no close node exists, search a close element */
    near3d(orig_ex,orig_ey,orig_ez,sort_ex,sort_ey,sort_ez,sort_enx,sort_eny,sort_enz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny, node[set[set1].node[i]].nz, tets, &near_node[0], n_closest_tets);
    //for (j=0; j<n_closest_tets; j++) printf("node:%d nearest tets:%d \n",set[set1].node[i], near_node[j]+1);

    etet=calcCoefficientsTet(set[set1].node[i], &near_node[0], n_closest_tets, node, tet, cof, extrapolflag);
    if(etet==-1)
    {
      if(coarseFlag) { coarseFlag=0; n_closest_tets*=10; goto secondLoop; }
      printf(" WARNING: no interpolation possible for node:%d\n", set[set1].node[i]);
      sem_wait(&sem_n);
      seta(failedNodesSet,"n",set[set1].node[i]);
      sem_post(&sem_n);
      continue;
    }
    //printf("node:%d near tet:%d \n",set[set1].node[i], etet+1);

    if((e_enqire[elem[tet[etet].e].nr].type==4)||(e_enqire[elem[tet[etet].e].nr].type==6))
    {
      //printf("create finer tets for elem:%d\n",elem[tet[etet].e].nr);
      e=elem[tet[etet].e].nr;
      elem2.type=e_enqire[e].type;
      elem2.nr=e;
      for (j=0; j<26; j++) elem2.nod[j]=e_enqire[e].nod[j];

      j = splitElementsToTets(1, nod, &elem2, &tet2);

      /* delete unusable tets (small volume) */
      e=0;
      for(ii=0; ii<j; ii++)
      {
        if(tet2[ii].v<SMALL_TET_VOLUME) { continue; }
        if(e<ii)
        {   
          for(n=0;n<4; n++) tet2[e].n[n]=tet2[ii].n[n];
          for(n=0;n<3; n++) tet2[e].cg[n]=tet2[ii].cg[n];
          tet2[e].v=tet2[ii].v;
          tet2[e].e=tet2[ii].e;
        }
        e++;
      }
      
      //printf(" %d finer tets created\n", e);
      for(j=0; j<e; j++) tetList[j]=j;
      etet=calcCoefficientsTet(set[set1].node[i], &tetList[0], e, node, tet2, cof, extrapolflag);
      //printf("closest tet:%d\n", etet);

      /* temporary switch the tet pointer to the tet2 pointer */
      tetbuf=tet;
      //printf("switch the tet pointer forth %d\n", tetbuf);
      tet=tet2;
    }
    else tetbuf=0;

    /* interpolate */
    if(allds)
    {
      /* all datasets should be interpolated */
      for(ds=0; ds<anz->l; ds++)
      {
        for(e=0; e<lcase[ds].ncomps; e++)
        {
          if(etet>=0)
          {
            //printf("ds:%d e:%d etet:%d \n",ds+1,e+1, etet);
            //for(j=0; j<4; j++) printf("n:%d val*cof: %f %f = %f\n",tet[etet].n[j], lcase[ds].dat[e][tet[etet].n[j]], cof[j], lcase[ds].dat[e][tet[etet].n[j]]*cof[j]);
            lcase[ds].dat[e][set[set1].node[i]]=0.;
            for(j=0; j<4; j++) lcase[ds].dat[e][set[set1].node[i]]+=lcase[ds].dat[e][tet[etet].n[j]]*cof[j];

            //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][set[set1].node[i]]);
          }
        }
      }
    }
    else
    {
      for(e=0; e<lcase[ds].ncomps; e++)
      {
        if(etet>-1)
        {
          //printf("ds:%d e:%d etet:%d \n",ds+1,e+1, etet);
          //for(j=0; j<4; j++) printf("n:%d val*cof: %f %f = %f\n",tet[etet].n[j], lcase[ds].dat[e][tet[etet].n[j]],cof[j], lcase[ds].dat[e][tet[etet].n[j]]*cof[j]);
          lcase[ds].dat[e][set[set1].node[i]]=0.;
          for(j=0; j<4; j++) lcase[ds].dat[e][set[set1].node[i]]+=lcase[ds].dat[e][tet[etet].n[j]]*cof[j];

          //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][set[set1].node[i]]);
        }
        
      }
    }
    if(tetbuf)
    {
      /* switch the tet pointer back */
      //printf("switch the tet pointer back %d\n", tetbuf);
      tet=tetbuf;
      tetbuf=0;
    }
  }

  return((void *)1);
}



/* return 0: ok, -1: no datasets, else setNr with nodes which could not be mapped */
int interpol3d(int set1, int set2, char *format, char *dataset, int extrapolflag)
{

#if NOTHREADING
  // from here on initialization w/o subroutine

  int i,ii,j,n,e;
  int ds=0, allds=0;
  int etet, tets=0, n1,n2, elnr, failedNodesSet, coarseFlag;
  double cof[4], dr, dx, dy, dz,tol;
  static Elements  *elem=NULL;   
  static Nodes     *nod=NULL;
  static Tetraeder *tet=NULL;
  static Rsort *rsort=NULL;

  static double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  static int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL, near_node[N_CLOSEST_TETS];
  int     n_closest_nodes;

  static double *orig_ex=NULL, *orig_ey=NULL, *orig_ez=NULL, *sort_ex=NULL, *sort_ey=NULL, *sort_ez=NULL;
  static int *sort_enx=NULL, *sort_eny=NULL, *sort_enz=NULL;
  int     n_closest_tets;

  static Elements  elem2;   
  static Tetraeder *tet2=NULL;
  static Tetraeder *tetbuf=0;
  static int tetList[MAX_NR_OF_TETS_PER_ELEM];

#else
  // initialization w subroutine for threading (less variables needed)
  int i,j,n,e;
  int ds=0, allds=0;
  int tets=0, n1,n2, elnr, failedNodesSet;
  double tol;
  Elements  *elem=NULL;   
  Nodes     *nod=NULL;
  Tetraeder *tet=NULL;
  Rsort *rsort=NULL;

  double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL;
  double *orig_ex=NULL, *orig_ey=NULL, *orig_ez=NULL, *sort_ex=NULL, *sort_ey=NULL, *sort_ez=NULL;
  int *sort_enx=NULL, *sort_eny=NULL, *sort_enz=NULL;

  typedef struct {
    Tetraeder *tet;
    Nodes     *nod;
    Elements  *elem;
    int vargi[9];
    double vargd[1];
    int *vargpi[6];
    double *vargpd[12];
  } Threadargs;
  Threadargs *targ=NULL;
  pthread_t *tid=NULL;
  int nlocalThreads;
  int threads=NTHREADS_MAX;
#endif

#if TEST
  double volu=0, cg[3]={0,0,0};
  FILE *handle;
#endif

  printf("interpol3d\n");
  tol=gtol;
  tol*=tol;

  if ( (nod = (Nodes *)realloc( (Nodes *)nod, (anz->nmax+1) * sizeof(Nodes))) == NULL )
  {
    printf("WARNING: realloc error interpol3d\n");
  }
  if((elem = (Elements *)realloc( (Elements *)elem, (set[set2].anz_e) * sizeof(Elements))) == NULL )
  {
    printf("WARNING: realloc error interpol3d\n");
  }
  for (j=1; j<=anz->nmax; j++)
  {
    nod[j].nx=node[j].nx;
    nod[j].ny=node[j].ny;
    nod[j].nz=node[j].nz;
  }
  for (i=0; i<set[set2].anz_e; i++)
  {
    e=set[set2].elem[i];
    if(e_enqire[e].type==4) elem[i].type=1;
    else if(e_enqire[e].type==6) elem[i].type=3;
    else elem[i].type=e_enqire[e].type;
    elem[i].nr=e;
    for (j=0; j<26; j++) elem[i].nod[j]=e_enqire[e].nod[j];
  }

  if(dataset[0]=='d')
  {
    if(compareStrings( dataset, "ds" )>0)
    {
      /* all datasets should be interpolated */
      for(ds=0; ds<anz->l; ds++)
      {
        for(e=0; e<lcase[ds].ncomps; e++)
          printf(" interpol:%s entity:%s\n", lcase[ds].name, lcase[ds].compName[e]);
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[ds].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , ds, anz, node, lcase )==-1) 
          {
            printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", ds+1); 
    	    return(-1);
          }
          calcDatasets( ds, anz, node, lcase );
          recompileEntitiesInMenu(ds);
        }
      }
      allds=1;
    }
    else
    {
      ds=atoi(&dataset[2])-1; 
      if((ds<0)||(ds>anz->l-1)) { printf(" specified Dataset:%d not available\n",ds); return(-1); }
      for(e=0; e<lcase[ds].ncomps; e++)
        printf(" interpol:%s entity:%s\n", lcase[ds].name, lcase[ds].compName[e]);
      /* check if the data of the specified lcase (Dataset) are already available */
      if (!lcase[ds].loaded)
      {
        if( pre_readfrdblock(copiedNodeSets , ds, anz, node, lcase )==-1) 
        {
          printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", ds+1); 
          return(-1);
        }
        calcDatasets( ds, anz, node, lcase );
        recompileEntitiesInMenu(ds);
      }
    }
  }
  else { printf(" Dataset not given for interpolation of data\n"); return(-1); }


  /* split elements into tet's (with cg and volu) */
  tets = splitElementsToTets(set[set2].anz_e, nod, elem, &tet);
  if(!tets) { printf(" No 3D master elements found\n"); return(set1); }

  /* delete unusable tets (small volume) */
  j=0;
  for(i=0; i<tets; i++)
  {
    if(tet[i].v<SMALL_TET_VOLUME) { if(printFlag) printf("Scip Tet:%d vol:%e < SMALL_TET_VOLUME\n", i, tet[i].v); continue; }
    if(j<i)
    {   
      for(n=0;n<4; n++) tet[j].n[n]=tet[i].n[n];
      for(n=0;n<3; n++) tet[j].cg[n]=tet[i].cg[n];
      tet[j].v=tet[i].v;
      tet[j].e=tet[i].e;
    }
    j++;
  }
  tets=j;
  if(printFlag) printf(" %d tets created\n", tets);

#if TEST
  handle = fopen ( "tets.inp", "w");
  fprintf(handle,"*NODE, NSET=Nall\n");
  for (i=0; i<anz->n; i++)
  {
    j=node[i].nr;
    //if( node[j].pflag == 0)
      fprintf(handle," %d,%f,%f,%f\n", j,node[j].nx,node[j].ny,node[j].nz  );
  }
  fprintf(handle,"*NODE, NSET=CG\n");
  for(i=0; i<tets; i++)
  {
    fprintf(handle," %d,%f,%f,%f\n", i+anz->nmax+1, tet[i].cg[0], tet[i].cg[1], tet[i].cg[2] );
    cg[0]+=tet[i].cg[0];
    cg[1]+=tet[i].cg[1];
    cg[2]+=tet[i].cg[2];
  }
  cg[0]/=tets;
  cg[1]/=tets;
  cg[2]/=tets;
  fprintf(handle,"*ELEMENTS, TYPE=C3D4\n");
  for(i=0; i<tets; i++)
  {
    fprintf(handle," %d,%d,%d,%d,%d\n", i+1, tet[i].n[0], tet[i].n[1], tet[i].n[2], tet[i].n[3]);
  }
  fprintf(handle,"*TEMPERATURE\nNall,0.\n");
  for(i=0; i<tets; i++)
  {
    fprintf(handle," %d,%e\n", i+anz->nmax+1,  tet[i].v );
    volu+=tet[i].v;
  }
  //printf("v:%f cg:%f %f %f\n",volu*scale->w*scale->w*scale->w,cg[0]* scale->w+scale->x,cg[1]* scale->w+scale->y,cg[2]* scale->w+scale->z);
  fclose(handle);
#endif

  /* extend the datasets by the midface nodes later used in the process */
  if(allds)
  {
    /* all datasets should be interpolated */
    for(ds=0; ds<anz->l; ds++)
    {
      for(e=0; e<lcase[ds].ncomps; e++)
      {
        if((lcase[ds].dat[e] = (float *)realloc(lcase[ds].dat[e], (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: realloc failure\n");
        /* block from nodalDataset() */
        for ( j=0; j<set[set2].anz_e; j++ )
        {
          elnr=set[set2].elem[j];
          switch(e_enqire[elnr].type)
          {
            case 4:
            for (n=0; n<3; n++)  /* create new vals at nodes in center of areas */
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[20+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[4+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[13+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[16+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[23]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[7]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[11]]+lcase[ds].dat[e][e_enqire[elnr].nod[12]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[19]]+lcase[ds].dat[e][e_enqire[elnr].nod[15]]) ;
            for (n=0; n<2; n++)  
            {
              n1=n*4;
              n2=n*8;
              lcase[ds].dat[e][e_enqire[elnr].nod[24+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n1]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[2+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n1]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[9+n2]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[10+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[11+n2]]) ;
            }
            break;
      
      
            case 5:
            for (n=0; n<2; n++) 
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[15+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[6+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[10+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[17]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[2]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[11]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[18]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[1]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 7]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[ 6]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 0]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[19]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[4]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5]]+lcase[ds].dat[e][e_enqire[elnr].nod[3]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[12]]+lcase[ds].dat[e][e_enqire[elnr].nod[13]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 3]]) ;
            break;
      
      
            case 10:
            lcase[ds].dat[e][e_enqire[elnr].nod[8]] = -0.25* (
            lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[1]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]])+0.5*(
            lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[6]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[7]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]]);
            break;
          }
        }
	  /* end block */
      }
    }
  }
  else
  {
    for(e=0; e<lcase[ds].ncomps; e++)
    {
        if((lcase[ds].dat[e] = (float *)realloc(lcase[ds].dat[e], (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: realloc failure\n");
        /* block from nodalDataset() */
        for ( j=0; j<set[set2].anz_e; j++ )
        {
          elnr=set[set2].elem[j];
          switch(e_enqire[elnr].type)
          {
            case 4:
            for (n=0; n<3; n++)  /* create new vals at nodes in center of areas */
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[20+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[4+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[13+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[16+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[23]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[7]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[11]]+lcase[ds].dat[e][e_enqire[elnr].nod[12]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[19]]+lcase[ds].dat[e][e_enqire[elnr].nod[15]]) ;
            for (n=0; n<2; n++)  
            {
              n1=n*4;
              n2=n*8;
              lcase[ds].dat[e][e_enqire[elnr].nod[24+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n1]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[2+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n1]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[9+n2]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[10+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[11+n2]]) ;
            }
            break;
      
      
            case 5:
            for (n=0; n<2; n++) 
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[15+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[6+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[10+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[17]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[2]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[11]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[18]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[1]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 7]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[ 6]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 0]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[19]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[4]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5]]+lcase[ds].dat[e][e_enqire[elnr].nod[3]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[12]]+lcase[ds].dat[e][e_enqire[elnr].nod[13]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 3]]) ;
            break;
      
      
            case 10:
            lcase[ds].dat[e][e_enqire[elnr].nod[8]] = -0.25* (
            lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[1]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]])+0.5*(
            lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[6]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[7]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]]);
            break;
          }
        }
	  /* end block */
    }
  }

  printf(" start interpolation\n");

  /* create set for failed nodes */
  delSet("-notMapped");
  failedNodesSet=pre_seta("-notMapped","i",0);

  if(set[set2].anz_n>tets)
  {
    if ( (rsort = (Rsort *)malloc( (set[set2].anz_n+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  }
  else
  {
    if ( (rsort = (Rsort *)malloc( (tets+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  }

  /* get the close node */
  /* search the closest node */
  if ( (orig_x = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_y = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_z = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_x = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_y = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_z = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_nx = (int *)malloc( (set[set2].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ny = (int *)malloc( (set[set2].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_nz = (int *)malloc( (set[set2].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  for(i=0; i<set[set2].anz_n; i++)
  {
    rsort[i].r=orig_x[i]=node[set[set2].node[i]].nx;
    rsort[i].i=i;
  }
  qsort( rsort, set[set2].anz_n, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<set[set2].anz_n; i++)
  {
    sort_x[i]=rsort[i].r;
    sort_nx[i]=rsort[i].i;
  }
  for(i=0; i<set[set2].anz_n; i++)
  {
    rsort[i].r=orig_y[i]=node[set[set2].node[i]].ny;
    rsort[i].i=i;
  }
  qsort( rsort, set[set2].anz_n, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<set[set2].anz_n; i++)
  {
    sort_y[i]=rsort[i].r;
    sort_ny[i]=rsort[i].i;
  }
  for(i=0; i<set[set2].anz_n; i++)
  {
    rsort[i].r=orig_z[i]=node[set[set2].node[i]].nz;
    rsort[i].i=i;
  }
  qsort( rsort, set[set2].anz_n, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<set[set2].anz_n; i++)
  {
    sort_z[i]=rsort[i].r;
    sort_nz[i]=rsort[i].i;
  }
  
  /* get the close tets */
  /* search the closest elements based on the closest cg */
  if ( (orig_ex = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_ey = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_ez = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ex = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ey = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ez = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_enx = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_eny = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_enz = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ex[i]=tet[i].cg[0];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ex[i]=rsort[i].r;
    sort_enx[i]=rsort[i].i;
  }
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ey[i]=tet[i].cg[1];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ey[i]=rsort[i].r;
    sort_eny[i]=rsort[i].i;
  }
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ez[i]=tet[i].cg[2];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ez[i]=rsort[i].r;
    sort_enz[i]=rsort[i].i;
  }

#if NOTHREADING

  n_closest_nodes=1;

  //while(1)
  for (i=0; i<set[set1].anz_n; i++ )
  {
    /* get the next node (i) */
    //if(sem_wait(&sem_map3d)) printf("Error in:sem_wait\n");
    //i=glob_map3d++;
    //if(sem_post(&sem_map3d)) printf("Error in:sem_post\n");
    //if(i>=param->vargi[1]) break;
    //printf(" nothread node:%d tol:%e\n", set[set1].node[i], tol);

    /* first, search for a close node */
    near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny,
           node[set[set1].node[i]].nz, set[set2].anz_n, &near_node[0], n_closest_nodes);
    //for (j=0; j<n_closest_nodes; j++) printf("node:%d near node:%d \n",set[set1].node[i], set[set2].node[near_node[j]]);
    dx= node[set[set2].node[near_node[0]]].nx - node[set[set1].node[i]].nx;
    dy= node[set[set2].node[near_node[0]]].ny - node[set[set1].node[i]].ny;
    dz= node[set[set2].node[near_node[0]]].nz - node[set[set1].node[i]].nz;      
    dr=dx*dx + dy*dy + dz*dz;      
    if(dr<tol )
    {
      if(printFlag) printf("node:%d, found equal node:%d with dr:%lf\n", set[set1].node[i], set[set2].node[near_node[0]], sqrt(dr));

      /* interpolate */
      if(allds)
      {
        /* all datasets should be interpolated */
        for(ds=0; ds<anz->l; ds++)
        {
          for(e=0; e<lcase[ds].ncomps; e++)
          {
            lcase[ds].dat[e][set[set1].node[i]]=lcase[ds].dat[e][ set[set2].node[near_node[0]] ];
            //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][ set[set1].node[i] ]);
          }
        }
      }
      else
      {
        for(e=0; e<lcase[ds].ncomps; e++)
        {
          lcase[ds].dat[e][set[set1].node[i]]=lcase[ds].dat[e][ set[set2].node[near_node[0]] ];
          //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][ set[set1].node[i] ]);
        }
      }
      continue;
    }

    if((int)N_CLOSEST_TETS/10<tets)  n_closest_tets= (int)N_CLOSEST_TETS/10; else n_closest_tets= tets;
    coarseFlag=1;
  secondLoop:;
    /* if no close node exists, search a close element */
    near3d(orig_ex,orig_ey,orig_ez,sort_ex,sort_ey,sort_ez,sort_enx,sort_eny,sort_enz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny, node[set[set1].node[i]].nz, tets, &near_node[0], n_closest_tets);
    //for (j=0; j<n_closest_tets; j++) printf("node:%d nearest tets:%d \n",set[set1].node[i], near_node[j]+1);

    etet=calcCoefficientsTet(set[set1].node[i], &near_node[0], n_closest_tets, node, tet, cof, extrapolflag);
    if(etet==-1)
    {
      if(coarseFlag) { coarseFlag=0; n_closest_tets*=10; goto secondLoop; }
      printf(" WARNING: no interpolation possible for node:%d\n", set[set1].node[i]);
      //sem_wait(&sem_g);
      seta(failedNodesSet,"n",set[set1].node[i]);
      //sem_post(&sem_g);
      continue;
    }
    //printf("node:%d near tet:%d \n",set[set1].node[i], etet+1);

    if((e_enqire[elem[tet[etet].e].nr].type==4)||(e_enqire[elem[tet[etet].e].nr].type==6))
    {
      //printf("create finer tets for elem:%d\n",elem[tet[etet].e].nr);
      e=elem[tet[etet].e].nr;
      elem2.type=e_enqire[e].type;
      elem2.nr=e;
      for (j=0; j<26; j++) elem2.nod[j]=e_enqire[e].nod[j];

      j = splitElementsToTets(1, nod, &elem2, &tet2);

      /* delete unusable tets (small volume) */
      e=0;
      for(ii=0; ii<j; ii++)
      {
        if(tet2[ii].v<SMALL_TET_VOLUME) { continue; }
        if(e<ii)
        {   
          for(n=0;n<4; n++) tet2[e].n[n]=tet2[ii].n[n];
          for(n=0;n<3; n++) tet2[e].cg[n]=tet2[ii].cg[n];
          tet2[e].v=tet2[ii].v;
          tet2[e].e=tet2[ii].e;
        }
        e++;
      }
      
      //printf(" %d finer tets created\n", e);
      for(j=0; j<e; j++) tetList[j]=j;
      etet=calcCoefficientsTet(set[set1].node[i], &tetList[0], e, node, tet2, cof, extrapolflag);
      //printf("closest tet:%d\n", etet);

      /* temporary switch the tet pointer to the tet2 pointer */
      tetbuf=tet;
      //printf("switch the tet pointer forth %d\n", tetbuf);
      tet=tet2;
    }
    else tetbuf=0;

    /* interpolate */
    if(allds)
    {
      /* all datasets should be interpolated */
      for(ds=0; ds<anz->l; ds++)
      {
        for(e=0; e<lcase[ds].ncomps; e++)
        {
          if(etet>=0)
          {
            //printf("ds:%d e:%d etet:%d \n",ds+1,e+1, etet);
            //for(j=0; j<4; j++) printf("n:%d val*cof: %f %f = %f\n",tet[etet].n[j], lcase[ds].dat[e][tet[etet].n[j]], cof[j], lcase[ds].dat[e][tet[etet].n[j]]*cof[j]);
            lcase[ds].dat[e][set[set1].node[i]]=0.;
            for(j=0; j<4; j++) lcase[ds].dat[e][set[set1].node[i]]+=lcase[ds].dat[e][tet[etet].n[j]]*cof[j];

            //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][set[set1].node[i]]);
          }
        }
      }
    }
    else
    {
      for(e=0; e<lcase[ds].ncomps; e++)
      {
        if(etet>-1)
        {
          //printf("ds:%d e:%d etet:%d \n",ds+1,e+1, etet);
          //for(j=0; j<4; j++) printf("n:%d val*cof: %f %f = %f\n",tet[etet].n[j], lcase[ds].dat[e][tet[etet].n[j]],cof[j], lcase[ds].dat[e][tet[etet].n[j]]*cof[j]);
          lcase[ds].dat[e][set[set1].node[i]]=0.;
          for(j=0; j<4; j++) lcase[ds].dat[e][set[set1].node[i]]+=lcase[ds].dat[e][tet[etet].n[j]]*cof[j];

          //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][set[set1].node[i]]);
        }
        
      }
    }
    if(tetbuf)
    {
      /* switch the tet pointer back */
      //printf("switch the tet pointer back %d\n", tetbuf);
      tet=tetbuf;
      tetbuf=0;
    }
  }

#else

#ifdef SEMINIT
  if(sem_init(&sem_map3d, 0, 1) < 0) printf("Error in:sem_init\n");
#else
  mptr = sem_open("/sem_map3d", O_CREAT, 0644, 1);
  sem_unlink("/sem_map3d");
  if((mptr == SEM_FAILED)) printf("Error in:sem_open\n");
#endif    
  
  if(threads>set[set1].anz_n) { nlocalThreads=set[set1].anz_n; }
  else nlocalThreads=threads;
  //nlocalThreads=1;
  if ((tid=(pthread_t *)realloc((pthread_t *)tid, nlocalThreads*sizeof(pthread_t)) ) == NULL ) { printf(" ERROR: malloc failure\n\n"); return(-1);}
  if ((targ=(Threadargs *)realloc((Threadargs *)targ, nlocalThreads*sizeof(Threadargs)) ) == NULL ) { printf(" ERROR: malloc failure\n\n"); return(-1);}

  for(i=0; i<nlocalThreads; i++)
  {
    targ[i].tet                   = tet;
    targ[i].nod                   = nod;
    targ[i].elem                 = elem;
    targ[i].vargd[0]              = tol;
    targ[i].vargi[0]                 =i;
    targ[i].vargi[1]   =set[set1].anz_n;
    targ[i].vargi[2]            = allds;
    targ[i].vargi[3]             = set1;
    targ[i].vargi[4]             = set2;
    targ[i].vargi[5]     = extrapolflag;
    targ[i].vargi[6]   = failedNodesSet;
    targ[i].vargi[7]             = tets;
    targ[i].vargi[8]               = ds;
    targ[i].vargpd[0]          = orig_x;
    targ[i].vargpd[1]          = orig_y;
    targ[i].vargpd[2]          = orig_z;
    targ[i].vargpd[3]          = sort_x;
    targ[i].vargpd[4]          = sort_y;
    targ[i].vargpd[5]          = sort_z;
    targ[i].vargpi[0]         = sort_nx;
    targ[i].vargpi[1]         = sort_ny;
    targ[i].vargpi[2]         = sort_nz;
    targ[i].vargpd[6]         = orig_ex;
    targ[i].vargpd[7]         = orig_ey;
    targ[i].vargpd[8]         = orig_ez;
    targ[i].vargpd[9]         = sort_ex;
    targ[i].vargpd[10]        = sort_ey;
    targ[i].vargpd[11]        = sort_ez;
    targ[i].vargpi[3]        = sort_enx;
    targ[i].vargpi[4]        = sort_eny;
    targ[i].vargpi[5]        = sort_enz;
    //thread_interpol3d((void *)&targ[i]);
    pthread_create(&tid[i],NULL,thread_interpol3d,(void *)&targ[i]);
  }
  for(i=0; i<nlocalThreads; i++)
  {
    pthread_join(tid[i], NULL);
  }
  glob_map3d=0;
  free(tid); tid=NULL;
  free(targ); targ=NULL;
#ifdef SEMINIT
  if(sem_destroy(&sem_map3d) < 0) printf("Error in:sem_init\n");
#else
  if(sem_close(mptr) < 0) printf("Error in:sem_close\n");
#endif    
#endif

  if(rsort) free(rsort);
  if(orig_x) free(orig_x);
  if(orig_y) free(orig_y);
  if(orig_z) free(orig_z);
  if(sort_x) free(sort_x);
  if(sort_y) free(sort_y);
  if(sort_z) free(sort_z);
  if(sort_nx) free(sort_nx);
  if(sort_ny) free(sort_ny);
  if(sort_nz) free(sort_nz);

  if(orig_ex) free(orig_ex);
  if(orig_ey) free(orig_ey);
  if(orig_ez) free(orig_ez);
  if(sort_ex) free(sort_ex);
  if(sort_ey) free(sort_ey);
  if(sort_ez) free(sort_ez);
  if(sort_enx) free(sort_enx);
  if(sort_eny) free(sort_eny);
  if(sort_enz) free(sort_enz);

  free(nod);
  free(elem);
  free(tet);
  nod=NULL;
  elem=NULL;

  printf("done\n");
  return(failedNodesSet);
}



int brecord( char *rec_str, char dat[MAX_PARAM_IN_BRECORD][MAX_LINE_LENGTH])
{
  int i, length;
  int nextarg=0, letter=0;

  length=strlen(rec_str);
  /* scan all args divided by comma */
  nextarg=0;letter=0;
  for(i=0; i<=length; i++)
  {
    if(rec_str[i]==(char)EOF) {dat[nextarg][letter]='\0';  break; } 
    if(rec_str[i]=='\n') {dat[nextarg][letter]='\0';  break; } 
    if(rec_str[i]==0) {dat[nextarg][letter]='\0';  break; } 
    if((rec_str[i]==' ')&&(letter))
    {
      dat[nextarg][letter]='\0';
      nextarg++;
      letter=0;
      if(nextarg>=MAX_PARAM_IN_BRECORD) { printf("ERROR: too much parameters in line:\n%s\n\n", rec_str); exit(1); }
    }
    else
    {
      if((rec_str[i]>32)&& (rec_str[i]<127))
      {
        dat[nextarg][letter]=rec_str[i];
        letter++;
        if(nextarg>=MAX_LINE_LENGTH) { printf("ERROR: brecord, increase MAX_LINE_LENGTH in extUtil.h\n\n"); exit(1); }
      }
    }
  }
  if(dat[nextarg][0]=='\0') return(nextarg);
  return(nextarg+1);
}



void pre_eprop(char *setname)
{
  int i,j,n,nr,ipuf,setNr;
  double dmin,dv[8][3],area,l,lmax,r,quality;

  int istat[3]={0,0,0};
  double vole, x[20],y[20],z[20];
  double xcge, ycge, zcge;
  double xl[20][3], cg[3],vn[3];
  char elty[MAX_LINE_LENGTH];

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" set:%s does not exist\n", setname);
    return;
  }
  printf (" create new dataset: EPROP for set:%s\n", set[setNr].name);

  /* create a new dataset */
  if ( (lcase = (Datasets *)realloc((Datasets *)lcase, (anz->l+1) * sizeof(Datasets))) == NULL )
    printf("\n\n ERROR: malloc failure\n\n" );

  lcase[anz->l].ncomps = 3;
  lcase[anz->l].irtype = 1;
  lcase[anz->l].npheader  = 0 ;
  lcase[anz->l].value  = 0. ;
  strcpy(lcase[anz->l].name,"EPROP") ;
  strcpy(lcase[anz->l].dataset_name,"");
  strcpy(lcase[anz->l].dataset_text,set[setNr].name);
  strcpy(lcase[anz->l].analysis_name,"");
  lcase[anz->l].step_number=-1;
  lcase[anz->l].analysis_type=1;
  lcase[anz->l].loaded = 1;
  lcase[anz->l].fileptr = NULL;

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
  for(i=0; i<lcase[anz->l].ncomps; i++)
  {
    if ( (lcase[anz->l].dat[i] = (float *)malloc( (anz->nmax+1) * sizeof(float))) == NULL )
      printf("\n\n ERROR: malloc failure\n\n" );	               
    if ( (lcase[anz->l].compName[i] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
       printf("\n\n ERROR: malloc failed\n\n" );
    if ( (lcase[anz->l].icname[i] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
       printf("\n\n ERROR: malloc failed\n\n" );
    lcase[anz->l].max[i]=-MAX_INTEGER;
    lcase[anz->l].min[i]=MAX_INTEGER;
    lcase[anz->l].nmax[i]=0;
    lcase[anz->l].nmin[i]=0;
    for(n=0; n<=anz->nmax; n++)
      lcase[anz->l].dat[i][n]=0.;
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
  for(i=0; i<lcase[anz->l].ncomps; i++)
  {
    lcase[anz->l].menu[i] = 1;
    lcase[anz->l].ictype[i] = 1;
    lcase[anz->l].icind1[i] = 1;
    lcase[anz->l].icind2[i] = 0;
    lcase[anz->l].iexist[i] = 0;
  }
  sprintf( lcase[anz->l].compName[0],"1/DMIN");
  sprintf( lcase[anz->l].compName[1],"1/VOLU");
  sprintf( lcase[anz->l].compName[2],"Flatness");

  /** store the minimum distance between a given node and its closest neighbor **/

  /* initialize */
  for(i=0; i<lcase[anz->l].ncomps; i++)
  {
    for(n=0; n<anz->n; n++)
    {
      lcase[anz->l].dat[i][node[n].nr]=0.;
    }
  }

  /* go over all elements  */
  ipuf=0;
  for(i=0; i<set[setNr].anz_e; i++)
  {
    /* store the edge length if lower than already stored */
    if (e_enqire[set[setNr].elem[i]].type == 1) ipuf = 8;  /* HEXA8 */
    else if (e_enqire[set[setNr].elem[i]].type == 4) ipuf = 8;  /* HEXA20 as HEXA8 */
    else if (e_enqire[set[setNr].elem[i]].type == 2) ipuf = 6;  /* PENTA6 */
    else if (e_enqire[set[setNr].elem[i]].type == 5) ipuf = 6;  /* PENTA15 as PENTA6*/
    else if (e_enqire[set[setNr].elem[i]].type == 3) ipuf = 4;  /* TET4 */
    else if (e_enqire[set[setNr].elem[i]].type == 6) ipuf = 4;  /* TET10 as TET4 */
    else if (e_enqire[set[setNr].elem[i]].type == 7) ipuf = 3;  /* TRI3  */
    else if (e_enqire[set[setNr].elem[i]].type == 8) ipuf = 3;  /* TRI6 as TRI3  */
    else if (e_enqire[set[setNr].elem[i]].type == 9) ipuf = 4;  /* QUAD4 */
    else if (e_enqire[set[setNr].elem[i]].type == 10) ipuf = 4; /* QUAD8 as QUAD4*/
    else if (e_enqire[set[setNr].elem[i]].type == 11) ipuf = 2; /* BEAM2 */
    else if (e_enqire[set[setNr].elem[i]].type == 12) ipuf = 2; /* BEAM3 as BEAM2 */
    else printf("elem %d of type %d not supported\n", set[setNr].elem[i],e_enqire[set[setNr].elem[i]].type);
    for (n=0; n<ipuf; n++)
    {
      if(n==ipuf-1) v_result(&node[e_enqire[set[setNr].elem[i]].nod[n]].nx, &node[e_enqire[set[setNr].elem[i]].nod[0]].nx,dv[n]);
      else v_result(&node[e_enqire[set[setNr].elem[i]].nod[n]].nx, &node[e_enqire[set[setNr].elem[i]].nod[n+1]].nx,dv[n]);
      dmin=1./(v_betrag(dv[n])*scale->w);
      if(dmin>lcase[anz->l].dat[0][e_enqire[set[setNr].elem[i]].nod[n]]) lcase[anz->l].dat[0][e_enqire[set[setNr].elem[i]].nod[n]]=dmin;
    }
    
    /* store the volume if lower than already stored  */
    /* code from pre_volu() */
    /*
     In addition 
     calculate the element quality. The measure used is proportional
     to the ratio of the longest edge divided by the radius of the
     inscribed sphere. The proportionality constant is such that
     the quality is 1 for an equilateral tetrahedron. For all other
     elements it exceeds 1. The bigger this number, the worse the
     quality
    */
    nr=set[setNr].elem[i];
    if(e_enqire[nr].type==1)
    {
      for(j=0; j<8; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D8");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      dmin=1./vole;
      for (j=0;  j<8; j++)
      {
        if(dmin>lcase[anz->l].dat[1][e_enqire[nr].nod[j]]) lcase[anz->l].dat[1][e_enqire[nr].nod[j]]=dmin;
      }
    }
    else if(e_enqire[nr].type==4)
    {
      for(j=0; j<12; j++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[j]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[j]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[j]].nz* scale->w+scale->z;
      }
      for(n=16; n<20; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      for(n=12; n<16; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      strcpy(elty,"C3D20");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      dmin=1./vole;
      for (j=0;  j<20; j++)
      {
        if(dmin>lcase[anz->l].dat[1][e_enqire[nr].nod[j]]) lcase[anz->l].dat[1][e_enqire[nr].nod[j]]=dmin;
      }
    }
    else if(e_enqire[nr].type==2)
    {
      for(j=0; j<6; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D6");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      dmin=1./vole;
      for (j=0;  j<6; j++)
      {
        if(dmin>lcase[anz->l].dat[1][e_enqire[nr].nod[j]]) lcase[anz->l].dat[1][e_enqire[nr].nod[j]]=dmin;
      }
    }
    else if(e_enqire[nr].type==5)
    {
      for(j=0; j<9; j++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[j]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[j]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[j]].nz* scale->w+scale->z;
      }
      for(n=12; n<15; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      for(n=9; n<12; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      strcpy(elty,"C3D15");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      dmin=1./vole;
      for (j=0;  j<15; j++)
      {
        if(dmin>lcase[anz->l].dat[1][e_enqire[nr].nod[j]]) lcase[anz->l].dat[1][e_enqire[nr].nod[j]]=dmin;
      }
    }
    else if(e_enqire[nr].type==3)
    {
      for(j=0; j<4; j++)
      {
        x[j]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      tetraeder_(&nr, istat, x,y,z, &vole, &xcge, &ycge, &zcge );
      dmin=1./vole;
      
      /*  calculating area of each face in the element */
      area=0.;
      for (n=0;  n<4; n++)
      {
        if(n==3) v_prod( dv[n], dv[0], vn );
	else v_prod( dv[n], dv[n+1], vn );
        area+=v_betrag(vn)*scale->w*scale->w;
      }
      area*=.5;

      // radius of the inscribed sphere
      r=3.*vole/area;

      // maximum edge length
      lmax=0;
      for (n=0;  n<4; n++)
      {
	l=v_betrag(dv[n]);
	if( lmax<l) lmax=l;
      }
      lmax*=scale->w;
      // quality, alpha=sqrt(6.)/12.=0.20412;
      if(r>0.) quality=0.20412*lmax/r;
      else quality=MAX_FLOAT;
      printf("Volu:%f Area:%f lmax:%f r:%f quality: %f\n", vole, area, lmax, r, quality);

      for (j=0;  j<4; j++)
      {
        if(dmin>lcase[anz->l].dat[1][e_enqire[nr].nod[j]]) lcase[anz->l].dat[1][e_enqire[nr].nod[j]]=dmin;
        if(quality>lcase[anz->l].dat[2][e_enqire[nr].nod[j]]) lcase[anz->l].dat[2][e_enqire[nr].nod[j]]=quality;
      }
    }
    else if(e_enqire[nr].type==6)
    {
      for(j=0; j<10; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D10");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      dmin=1./vole;
      
      /*  calculating area of each face in the element */
      area=0.;
      for (n=0;  n<4; n++)
      {
        if(n==3) v_prod( dv[n], dv[0], vn );
	else v_prod( dv[n], dv[n+1], vn );
        area+=v_betrag(vn)*scale->w*scale->w;
      }
      area*=.5;

      // radius of the inscribed sphere
      r=3.*vole/area;

      // maximum edge length
      lmax=0;
      for (n=0;  n<4; n++)
      {
	l=v_betrag(dv[n]);
	if( lmax<l) lmax=l;
      }
      lmax*=scale->w;
      // quality, alpha=sqrt(6.)/12.=0.20412;
      if(r>0.) quality=0.20412*lmax/r;
      else quality=MAX_FLOAT;
      // printf("Volu:%f Area:%f lmax:%f r:%f quality: %f\n", vole, area, lmax, r, quality);

      for (j=0;  j<10; j++)
      {
        if(dmin>lcase[anz->l].dat[1][e_enqire[nr].nod[j]]) lcase[anz->l].dat[1][e_enqire[nr].nod[j]]=dmin;
        if(quality>lcase[anz->l].dat[2][e_enqire[nr].nod[j]]) lcase[anz->l].dat[2][e_enqire[nr].nod[j]]=quality;
      }
    }
    else
    {
      printf("ERROR: type:%d of elem:%d not known. Interrupt\n",e_enqire[nr].type,nr); 
      return;
    }
  }     

  /* search max min */
  for(i=0; i<lcase[anz->l].ncomps; i++)
  {
    for(n=0; n<anz->n; n++)
    {
      if (lcase[anz->l].dat[i][node[n].nr] >  lcase[anz->l].max[i])
      {  lcase[anz->l].max[i]=lcase[anz->l].dat[i][node[n].nr]; lcase[anz->l].nmax[i]=node[n].nr;}
      if (lcase[anz->l].dat[i][node[n].nr] <  lcase[anz->l].min[i])
      {  lcase[anz->l].min[i]=lcase[anz->l].dat[i][node[n].nr]; lcase[anz->l].nmin[i]=node[n].nr;}
    }
  }

  anz->l++;
  calcDatasets( anz->l-1, anz, node, lcase );
  recompileEntitiesInMenu(anz->l-1);
  if(activWindow!=-1) createDatasetEntries();
}


int pre_nurs(char *string, int addFlag)
{
  int i,ii,j,jj,k,n,m,s;
  int length, ps, nr, setNr, ibuf, sum_div;
  char  name[MAX_LINE_LENGTH], comm[MAX_LINE_LENGTH], buffer[MAX_LINE_LENGTH];
  int  lnew[2], cl[2];
  char typnew[2];
  int edge[MAX_EDGES_PER_SURF];                 /* lines/lcmb for the meshable substitute surface with 4 edges */
  char  ctyp[MAX_EDGES_PER_SURF];      /*   type: l=line c=lcmb */
  char  cori[MAX_EDGES_PER_SURF];      /*   l-orient +- */

  length = sscanf( string, "%s%s%s", name, comm, buffer  );

  /* check if a set of blended surfaces should make nurbs. The surfaces will use this nurbs */ 
  if((length==2) && ( name[0] == '!' ))
  {
    setNr=getSetNr(comm);
    if(setNr<0)
    {
      printf("ERROR: Set:[%s] does not exist\n",comm);
      return(-2);
    }
    setall=getSetNr("all");
    /* try to create a nurbs if no shape is assigned */
    ibuf=set[setNr].anz_s;
    for ( i=0; i<ibuf; i++)
    {
      nr = set[setNr].surf[i];
      if (surf[nr].sh<=-1)
      {
        /* nurbs need 4 edges, create a temporary surf if the requirement is violated */
        if (surf[nr].nl==4)
	{
          s=nr;
          k=createBlendedNurbs(s);
	}
        else if (surf[nr].nl==2)
        {
          ps=splitLineAtDivratio( surf[nr].l[0], surf[nr].typ[0], 0.5, edge, ctyp);
          if (ps==-1) { return(-1); }
          if(surf[nr].o[0]=='-')
          {
            n=edge[0];
            m=ctyp[0];
            edge[0]=edge[1];
            ctyp[0]=ctyp[1];
            edge[1]=n;
            ctyp[1]=m;
          }
          ps=splitLineAtDivratio( surf[nr].l[1], surf[nr].typ[1], 0.5, &edge[2], &ctyp[2]);
          if (ps==-1) { return(-1); }
  
          /* create a 4 sided surf */
          for (jj=0; jj<4; jj+=2) { cori[jj]=cori[jj+1]=surf[nr].o[jj]; }
          getNewName( name, "s" );
          s=surface_i( name, surf[nr].ori, surf[nr].sh, (int)4, cori, edge, ctyp );
          if( s<0) { printf("ERROR: surface could not be created\n"); return(-1); }
          if(printFlag) printf(" temp. surf[%d]:%s generated\n",s, surf[s].name);
          k=createBlendedNurbs(s);
          delLine( 4, &edge[0] );
          delSurf( 1, &s );
        }
        else if (surf[nr].nl==3)
        {
          /* choose an edge with has no rest for the equation division/4 in case elements have quadratic formulation */
          for (ii=0; ii<3; ii++)
          {
            if(( surf[nr].typ[ii]=='l')&&(!(line[surf[nr].l[ii]].div%4))) break;
            else if( line[surf[nr].l[ii]].typ=='c')
            {
              sum_div=0;
              for(jj=0; jj<lcmb[surf[nr].l[ii]].nl; jj++) sum_div+=line[lcmb[surf[nr].l[ii]].l[jj]].div;
              if (!(sum_div%4)) break;
  	    }
          }
          /* no mesh if no suitable edge exists */
          if(ii==3) ii=2;
          ps=splitLineAtDivratio( surf[nr].l[ii], surf[nr].typ[ii], 0.5, lnew, typnew);
          if (ps==-1) { return(-1); }
  
          /* create a 4 sided surf */
          for (jj=0; jj<ii; jj++)
          {
            edge[jj]=surf[nr].l[jj];
            cori[jj]=surf[nr].o[jj];
            ctyp[jj]=surf[nr].typ[jj];
          }
          if(surf[nr].o[ii]=='+') { n=0; m=1; } else { n=1; m=0; }
          edge[jj]=lnew[n];
          cori[jj]=surf[nr].o[ii];
          ctyp[jj]=typnew[n];
          jj++;
          edge[jj]=lnew[m];
          cori[jj]=surf[nr].o[ii];
          ctyp[jj]=typnew[m];
          for (; jj<3; jj++) /* thats ok so */
          {
            edge[jj+1]=surf[nr].l[jj];
            cori[jj+1]=surf[nr].o[jj];
            ctyp[jj+1]=surf[nr].typ[jj];
          }
          getNewName( name, "s" );
          s=surface_i( name, surf[nr].ori, surf[nr].sh, (int)4, cori, edge, ctyp );
          if( s<0)
          { printf("ERROR: surface could not be created\n"); return(-1); }
          if(printFlag) printf(" temp. surf[%d]:%s generated\n",s, surf[s].name);
          k=createBlendedNurbs(s);
          delLine( 2, &lnew[0] );
          delSurf( 1, &s );
        }
        /* to mesh a n-sided surf two corners will be combined to one lcmb until only 4 edges remain */
        else if (surf[nr].nl>4)
        {
          /* create a temporary surface for the meshing */
          getNewName( name, "s" );
          s=surface_i( name, surf[nr].ori, surf[nr].sh, surf[nr].c[0], surf[nr].o, surf[nr].l, surf[nr].typ );
          if( s<0) { printf("ERROR: surface could not be created\n"); return(-1); }
          surf[s].etyp=surf[nr].etyp;
          surf[s].eattr=surf[nr].eattr;
  
          for (ii=0; ii<surf[nr].nl-4; ii++)
          {
            /* determine the best suited corner for the lcmb */
            determineBestCorners( s,cl);
            /* create a lcmb out of 2 edges */
            lnew[0]=addTwoLines( surf[s].l[cl[0]], surf[s].o[cl[0]], surf[s].typ[cl[0]], surf[s].l[cl[1]] ,surf[s].o[cl[1]], surf[s].typ[cl[1]] );
            if( lnew[0]==-1) { return(-1); }
  
            /* replace the concatonated lines of the surface */
            if (cl[0]==surf[s].nl-1)
            {
              surf[s].l[0]=lnew[0];
              surf[s].o[0]='+';
              surf[s].typ[0]='c';
              surf[s].nl--;
            }
            else
            {
              surf[s].l[cl[0]]=lnew[0];
              surf[s].o[cl[0]]='+';
              surf[s].typ[cl[0]]='c';
              for (jj=cl[0]+1; jj<surf[s].nl-1; jj++)
              {
                surf[s].l[jj]=surf[s].l[jj+1];
                surf[s].o[jj]=surf[s].o[jj+1];
                surf[s].typ[jj]=surf[s].typ[jj+1];
              }
              surf[s].nl--;
            }
          }
          if(printFlag) printf(" temp. surf[%d]:%s generated\n",s, surf[s].name);
          k=createBlendedNurbs(s);
          delSurf( 1, &s );
        }
        else return(-1);
        if(k>-1)
        {
          seta( setall, "S", k );
          for (j=0; j<anz->sets; j++)
          {
            if(( set[j].name != (char *)NULL)&&( set[j].flag=='o')) seta( j, "S", k );
          }

          /* create a shape of the same name for reference in surfaces */
          surf[nr].sh=shape_i( nurbs[k].name, 4, k, 0, 0, 0, 0, 0, 0);
          repNurs(k);
          repSurf(nr,0);
	}
      }
      for (i=0; i<anz->sets; i++)
      {
        if(( set[i].name != (char *)NULL)&&( set[i].flag=='o'))
        {
          seta( i, "sh", surf[nr].sh );
        }
       }
     }
  
  }
  else { if(nurs(string, addFlag)==-1) return(-1); }
  
  return(0);
}


void placeTxt(char *string)
{
  int   i,j, k, n=0, nr;
  double x,y;
  int args, ibuf, setNr=-1, dx,dy;
  GLint    viewport[4];
  GLdouble mvmatrix[16], projmatrix[16];
  char *text=NULL, name[MAX_LINE_LENGTH], dat[3][MAX_LINE_LENGTH];
  static GLdouble wx, wy, wz;  /*  returned window x, y, z coords  */
  static int flag;

  /* search a text between "" and erase him from the string */
  i=j=0;
  do
  {
    if(string[i]=='"')
    {
      string[i++]=' ';
      if((text=(char *)malloc(sizeof(char))) == NULL)
        printf("\n ERROR: malloc failed\n");
      while((string[i]!='"')&&(string[i]!=0))
      {
        text[j++]=string[i];
        string[i++]=' ';
        if((text=(char *)realloc(text,(j+1)*sizeof(char))) == NULL)
          printf("\n ERROR: realloc failed\n");
      }
      string[i++]=' ';
      text[j++]='\0';
    }
  }while(string[i++]!='\0');

  args=sscanf(string,"%s %s %s %s", name, dat[0], dat[1], dat[2]);
  if(checkIfNumber(name)) { n=atoi(name); }
  if(n<1)
  {
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" txt: %s does not exist\n", name);
      return;
    }
  }
  if(args==1) { dx=0; dy=0; k=-1; }
  else if(checkIfNumber(dat[0])) { x=atof(dat[0]); y=atof(dat[1]); k=-1; dx=x*width_w1; dy=y*height_w1; }
  else if(args==2) { dx=0; dy=0; k=0; }
  else { x=atof(dat[1]); y=atof(dat[2]); k=0; dx=x*width_w1; dy=y*height_w1; }

  if(setNr<0)
  {
    ibuf=0; i=set[ibuf].anz_n;
    goto skipSet;
  }
  else ibuf=setNr;
  for(i=0; i<set[ibuf].anz_n; i++)
  {
    n=set[ibuf].node[i];
  skipSet:;
    if(args<=2) nr=createText(n, -1, -1 );
    else nr=createText(n, -1, 0 );
    ntext[nr].text=text;
    ntext[nr].tFlag=1;
    if(k>-1)
    {
      for(j=0; j<strlen(dat[k]); j++)
      {
        if(dat[k][j]=='n') ntext[nr].nFlag=0;
        else if(dat[k][j]=='t') ntext[nr].tFlag=0;
        else if(dat[k][j]=='v') ntext[nr].vFlag=0;
        else if(dat[k][j]=='e') ntext[nr].fFlag=0;
        else if(dat[k][j]=='f') ntext[nr].fFlag=1;
        else if(dat[k][j]=='s') ntext[nr].pFlag=1;
        else if(dat[k][j]=='i') ntext[nr].fFlag=2;
      }
    }
    if((dx)||(dy))
    {
      glutSetWindow( w1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();
      moveModel();
    
      glGetIntegerv (GL_VIEWPORT, viewport);
      glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
      glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
      flag=gluProject( node[ntext[nr].node_nr].nx, node[ntext[nr].node_nr].ny, node[ntext[nr].node_nr].nz, mvmatrix, projmatrix, viewport,  &wx, &wy, &wz);
      if (flag==GL_FALSE) printf("WARNING: Malfunction, please reselect\n");
      ntext[nr].tx= wx+(double)(dx);
      ntext[nr].ty= (viewport[3]-wy)-(double)(dy);
    }
  }
}


void fixBadDivisions(int setNr)
{
  int i,j,f,c,s,l,k,n, ii;
  int surfbuf[10], nsurfs,counter,loops=0;
  int msh,se,nr;
  char buffer[MAX_LINE_LENGTH];

  loopFix:;

  // search for faces of type 11 and 12 which exists at unconnected triangles
  // and collect the related lines in set '-EDGE‘
  delSet(specialset->tmp);
  delSet("-mesh");
  delSet("-EDGE" );
  se=pre_seta(specialset->tmp,"i",0);
  msh=pre_seta("-mesh","i",0);
  nr=pre_seta("-EDGE","i",0);

  for (i=0; i<set[setNr].anz_b; i++) seta( se, "b", set[setNr].body[i] );
  for (i=0; i<set[setNr].anz_s; i++) seta( se, "s", set[setNr].surf[i] );

  /* cyrcle through all bodys and add all surfs */
  for (i=0; i<set[se].anz_b; i++)
  {
    c=set[se].body[i];
    for (j=0; j<body[c].ns; j++)
    {
      l=body[c].s[j];
      seta( se, "s", l);
    }
  }
  /* cyrcle through all surfs and add all lines, lcmbs and nurbs */
  for (i=0; i<set[se].anz_s; i++)
  {
    s=set[se].surf[i];
    for (j=0; j<surf[s].nl; j++)
    {
      l=surf[s].l[j];
      if(surf[s].typ[j]=='l')
      {
        seta( se, "l", l );
      }
      else
      {
        seta( se, "c", l );
      }
    }
  }
  /* cyrcle through all lcmbs and add all lines */
  for (i=0; i<set[se].anz_c; i++)
  {
    c=set[se].lcmb[i];
    for (j=0; j<lcmb[c].nl; j++)
    {
      l=lcmb[c].l[j];
      seta( se, "l", l);
    }
  }

  /* cyrcle through all lines and add all related faces of type 11,12 */
  for (i=0; i<set[se].anz_l; i++)
  {
    l= set[se].line[i];
    for(f=0; f<anz->f; f++)
    {
      counter=0;
      if((face[f].type==11)||(face[f].type==12))
      {
        for(n=0; n<2; n++)
        {
          for(k=0; k<line[l].nn; k++) if(line[l].nod[k]==face[f].nod[n]) counter++;
          if((point[line[l].p1].nn)&&(point[line[l].p1].nod[0]==face[f].nod[n])) counter++;
          if((point[line[l].p2].nn)&&(point[line[l].p2].nod[0]==face[f].nod[n])) counter++;
	}
      }
      if(counter==2)
      {
        seta(nr,"l",l);
      }
    }
  }
  if(set[nr].anz_l==0) goto exitfixBadDivisions;
  if(loops>=MAX_REFINEMENT_LOOPS)
  {
    delSet(specialset->tmp);
    delSet("-mesh");
    printf(" WARNING: mesh not closed (see set -EDGE) \n");
    return;
  }
  loops++;

  /* remesh the affected regions */
  /* add the surfaces */
  for(i=0; i<set[nr].anz_l; i++)
  {
    nsurfs=0;
    l=set[nr].line[i];
    // search for line related surfs
    for(k=0; k<set[se].anz_s; k++)
    {
      s=set[se].surf[k];
      for(ii=0; ii<surf[s].nl; ii++)
      {
        if(surf[s].l[ii]==l) surfbuf[nsurfs++]=s;
      }
    }
    // search for lcmb related surfs
    for(j=0; j<set[se].anz_c; j++)
    {
      c=set[se].lcmb[j];
      for (n=0; n<lcmb[c].nl; n++)
      {
        if(lcmb[c].l[n]==l)
        {
          for(k=0; k<set[se].anz_s; k++)
          {
            s=set[se].surf[k];
            for(ii=0; ii<surf[s].nl; ii++)
            {
              if(surf[s].l[ii]==lcmb[c].l[n]) surfbuf[nsurfs++]=s;
            }
          }
        }
      }
    }
    // add surfs only if they are '2' and of type unstruct tr3
    if(nsurfs==2)
    {
      k=0;
      for(j=0; j<2; j++)
      {
        s=surfbuf[j];
        if(((surf[s].etyp==7)||(surf[s].etyp==8))&&(surf[s].eattr==-1)) k=1;
      }
      if(k)
      {
        seta( msh, "s", surfbuf[0]);
        seta( msh, "s", surfbuf[1]);
        seta( msh, "l", l);
      }
    }
  }

  if(set[msh].anz_s)
  {
    // inc line div
    for(i=0; i<set[msh].anz_l; i++)
    {
      line[set[msh].line[i]].div+=2;
      repLine(set[msh].line[i]);
    }
  
    /* delete the mesh */
    sprintf(buffer,"me %s", set[msh].name);
    pre_del(buffer);
    for(i=0; i<set[msh].anz_l; i++)
    {
      //printf("del mesh line %s\n",line[set[msh].line[i]].name);
      line[set[msh].line[i]].nn=0;
      line[set[msh].line[i]].ne=0;
    }
    for(i=0; i<set[msh].anz_s; i++)
    {
      //printf("del mesh surf %s\n",surf[set[msh].surf[i]].name);
      surf[set[msh].surf[i]].nn=0;
      surf[set[msh].surf[i]].ne=0;
    }
    /* mesh again */
    for(i=0; i<set[msh].anz_s; i++)
    {
      printf("mesh %s\n",surf[set[msh].surf[i]].name);
    }
    completeSet(set[msh].name, "do");
    pre_mesh( set[msh].name );
    sprintf(buffer,"n %s",set[msh].name);
    pre_merge(buffer);
    printf(" %d edges in the model\n", anz->g);
  }
  else goto exitfixBadDivisions;
  goto loopFix;

 exitfixBadDivisions:;
  delSet( "-EDGE" );
  delSet(specialset->tmp);
  delSet("-mesh");
  return;
}




/*------------------------------------------------------------------*/
/* Daten im fbd-format einlesen                                     */
/*------------------------------------------------------------------*/
/* return <0 if reading has to be stopped abnormally */
int commandoInterpreter( char *type, char *string, int na, int nb, FILE *handle1, int addFlag, int *gtolFlag )
{
  int   i,j,k,l,n; 
  int   lc,nr,ibuf=0;
  char  cbuf;
  char  buffer[MAX_LINE_LENGTH];
  char  setname[MAX_LINE_LENGTH], setname2[MAX_LINE_LENGTH];
  char  xbuf[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH], dataset[MAX_LINE_LENGTH];
  char  name[MAX_LINE_LENGTH], comm[MAX_LINE_LENGTH], posn[MAX_LINE_LENGTH], entity[MAX_LINE_LENGTH];
  char  addDispFlagLocal=0;
  static char  **seq=NULL;
  char  **buf;
  char  *sbuf;
  char  dat[MAX_PARAM_IN_BRECORD][MAX_LINE_LENGTH];
  int       args,anz_seq, length, setNr, setNr2;
  double    lfbuf, pscal, x,y,z,dx,dy,dz, wx[2], wy[2], cof[4];
  char      **substring=NULL;
  int       foundSubString;
  static int *picnr=NULL;
  int pics;
  int pnts[3];
  DIR *dirp;
  struct dirent *dp;
  static int ifFlag=1, whileFlag=1, grpSets=0;
  int   returnFlag=1;
  Scale scaleSet[1];

  int ncomps, analysis_type, step_number;
  char dataset_text[MAX_LINE_LENGTH], analysis_name[MAX_LINE_LENGTH];
  static char *ptr_string=NULL;

  for(j=0;j<strlen(type); j++) type[j]=toupper(type[j]);
 whileLoop:;
  for(j=0;j<MAX_PARAM_PER_RECORD; j++) dat[j][0]=0;
  for(j=0;j<strlen(string); j++) if(string[j]=='\n') string[j]=0;

  /* check if a loop starts due to "WHILE" */ 
  /* this has to be evaluated before a substitution of values happened! */
  /* whileFlag=0 condition was not fulfilled, skip the command
     whileFlag=1 execute command
     whileFlag=2 store and execute command
     whileFlag=3 restore and execute command and at the end of commandoInterpreter() go back to whileLoop:
     whileFlag=4 store command but do not execute
     whileFlag=5 restore command but do not execute command and at the end of commandoInterpreter() go back to whileLoop:
  */
  //printf(" commandoInterpreter, string:%s\n", string);
  if(whileFlag>1)
  {
    if((whileFlag==3)||(whileFlag==5))
    {
      whileFlag=pre_while(type, &ptr_string, &na, &nb, handle1, &addFlag, gtolFlag, whileFlag,ifFlag);
      string=ptr_string;
    }
    else whileFlag=pre_while(type, &string, &na, &nb, handle1, &addFlag, gtolFlag, whileFlag,ifFlag);
  }
  // fill data-structures for while loops
  if( (compareStrings(type, "WHILE")>0)||(compareStrings(type, "ENDWHILE")>0))
  {
    if((whileFlag==3)||(whileFlag==5))
    {
      whileFlag=pre_while(type, &ptr_string, &na, &nb, handle1, &addFlag, gtolFlag, -whileFlag,ifFlag);
      string=ptr_string;
    }
    else whileFlag=pre_while(type, &string, &na, &nb, handle1, &addFlag, gtolFlag, -whileFlag,ifFlag);
    goto checkForError;
  }
  //printf("whileFlag:%d\n",whileFlag);
  if((whileFlag==0)||(whileFlag==4)||(whileFlag==5)) goto checkForError;


  /* replace parameters by defined values */
  //if ((compare(type, "VALU", 4)!=4)&&(compare(type, "PRNT",4)!=4)&&(compare(type, "SET", 3)!=3))
  if (compare(type, "VALU", 4)!=4)
  {
    args=brecord(&string[na], dat);
    string[na]=0;
    for(i=0; i<args; i++)
    {
      j=getValuNr(dat[i]);
      if (j>-1)
      {
        //printf (" param:%s replaced by value:%s\n", value[j].name, value[j].string );
        strcpy(dat[i],value[j].string );
      }
      sprintf(&string[strlen(string)]," %s",dat[i]);
    }
    //printf("replaced parameters by defined values: %s\n", string);
  }
  /* substitution of values in PRNT leads to errors!  */

  /* check for "IF" statements */ 
  if((compareStrings(type, "IF")>0)||(compareStrings(type, "ELSE")>0)||(compareStrings(type, "ENDIF")>0))
  {
    ifFlag=pre_if(string);
    goto checkForError;
  }
  if(ifFlag==0) goto checkForError;


  setname[0]=name[0]=posn[0]=comm[0]=entity[0]=xbuf[0]=format[0]=0;

  /* remove masking of parameters (usually values) */
  if((anz->v)&&(strlen(string)>0)) for(i=0; i<strlen(string)-1; i++) if(string[i]=='\\') { string[i]=' '; i+=2; } // check only every third char 

  if (compareStrings(type, "ALLOW_SYS")>0);
  else if (compareStrings(type, "ANIM")>0)
  {
    pre_animate(&string[na+1]);
  }
  else if (compareStrings(type, "AREA")>0)
  {
    pre_area(&string[na+1]);
  }
  else if (compareStrings(type, "ASGN")>0)
  {
    length= strsplt( &string[na+1], ' ', &buf);
    if(length>0) strcpy( entity, buf[0]);
    if(length>1) strcpy( name, buf[1]);
    /* free buf */
    for(i=0; i<length; i++) free(buf[i]);
    free(buf);
    
    if(( entity[0]=='s')&&( entity[1]=='e')) lchar->se=name[0];
    else if( compareStrings(entity, "bg")>0)
    {
      if( compare( name, "off", 3)==3) { if(backgroundFlag) {  backgroundFlag=0; inpformat=inpformatbuffer;} }
      else if( compare( name, "on", 2)==2)  { if(backgroundFlag==0) { backgroundFlag=1; inpformatbuffer=inpformat; inpformat=0; } }
    }
    else if( compareStrings(entity, "graph")>0)
    {
      if( compare( name, "off", 3)==3) graph_on=0;
      else if( compare( name, "on", 2)==2) graph_on=1;
      else graph_Nr=atoi(name)-1;
    }
    else if( compareStrings(entity, "cgxhelpfile")>0)
    {
      sprintf(helpfile[0],"\"%s\"",name);
    }
    else if( compareStrings(entity, "ccxhelpfile")>0)
    {
      sprintf(helpfile[1],"\"%s\"",name);
    }
#if defined AFLIB
    else if( compareStrings(entity, "aflibhelpfile")>0)
    {
      sprintf(helpfile[2],"\"%s\"",name);
    }
#endif
    else if( compareStrings(entity, "browser")>0)
    {
#ifdef WIN32
      sprintf(browser,"%s",name);
#else
      sprintf(browser,"\"%s\"",name);
#endif
    }
    else if( compareStrings(entity, "viewer")>0)
    {
#ifdef WIN32
      sprintf(psviewer,"%s",name);
#else
      sprintf(psviewer,"\"%s\"",name);
#endif
    }
    else if( compareStrings(entity, "viewformat")>0)
    {
      sprintf(viewformat,"%s",name);
    }
    else if( compareStrings(entity, "usr")>0)
    {
      createUSERparam( name, &string[na+3+strlen(entity)+strlen(name)]);
    }
    else if( compareStrings(entity, "netgen")>0)
    {
      meshp.tetmesher=0;
      printf(" tetmesher: netgen\n");
    }
    else if( compareStrings(entity, "tetgen")>0)
    {
      meshp.tetmesher=1;
      printf(" tetmesher: tetgen\n");
    }
    else if( compareStrings(entity, "mem")>0)
    {
      if(name[0]=='f')
      {
	anz->free=1;
        for(lc=0; lc<anz->l; lc++)
        {
          if(cur_lc!=lc)
          {
            //printf(" free lc[%d] ncomps:%d\n",lc,lcase[lc].ncomps);
            if(lcase[lc].loaded)
            {
              for(i=0; i<lcase[lc].ncomps; i++) free(lcase[lc].dat[i]);
            }
            /* always allocated */
            free(lcase[lc].dat);
            lcase[lc].dat=NULL;
            lcase[lc].loaded=0;
          }
        }
      }
      else anz->free=0;
    }
    else if( compareStrings(entity, "thrds")>0) anz->threads=atoi(name);
    else if( compareStrings(entity, "alpha")>0) meshp.alpha=atof(name);
    else if( compareStrings(entity, "beta")>0) meshp.beta=atof(name);
    else if( compareStrings(entity, "mpc")>0) { nasMpc=1; printf("rbe %d %f %8f %s\n", nasMpc, nasRbeHec, nasRbeHec, name); }

    else if( compareStrings(entity, "nadapt")>0) meshp.nadapt=atoi(name);
    else if( compareStrings(entity, "rbe")>0) { nasMpc=0; nasRbeHec=atof(name); printf("rbe %d %f %8f %s\n", nasMpc, nasRbeHec, nasRbeHec, name); }
    else if( entity[0]=='n') { if(atoi(name)>0) anz->nnext=atoi(name); else printf("ERROR: %s not a valid nr\n",name); }
    else if( entity[0]=='e') { if(atoi(name)>0) anz->enext=atoi(name); else printf("ERROR: %s not a valid nr\n",name); }
    else if( entity[0]=='p') lchar->p=name[0];
    else if( entity[0]=='l') lchar->l=name[0];
    else if( entity[0]=='c') lchar->c=name[0];
    else if( entity[0]=='s') lchar->s=name[0];
    else if( entity[0]=='b') lchar->b=name[0];
    else if( entity[0]=='L') lchar->L=name[0];
    else if( entity[0]=='S') lchar->S=name[0];
    else if( entity[0]=='h') lchar->sh=name[0];
    else if( compareStrings(entity, "max")>0) { defScalMethod=0; redraw(); }
    else if( compareStrings(entity, "maxr")>0) { defScalMethod=1; redraw(); }
    else if( compareStrings(entity, "maxc")>0)
    {
      defScalMethod=2;
      for(i=0; i<entitycols; i++) if(compareStrings(name,entitycol[i].name)>0) col_maxc=i;
      redraw();
    }
    else if( compareStrings(entity, "minc")>0)
    {
      defScalMethod=2;
      for(i=0; i<entitycols; i++) if(compareStrings(name,entitycol[i].name)>0) col_minc=i;
      redraw();
    }
    else errMsg(" %s not recognized\n", entity);
  }
  else if (compareStrings(type, "AVER")>0)
  {
    sscanf(&string[na+1], "%s", name); 
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" WARNING: set '%s' does not exist\n", name);
      goto checkForError;
    }
    x=y=z=0.;
    for (i=0; i<set[setNr].anz_p; i++)
    {
      x+=point[set[setNr].pnt[i]].px*scale->w+scale->x;
      y+=point[set[setNr].pnt[i]].py*scale->w+scale->y;
      z+=point[set[setNr].pnt[i]].pz*scale->w+scale->z;
    }
    for (i=0; i<set[setNr].anz_n; i++)
    {
      x+=node[set[setNr].node[i]].nx*scale->w+scale->x;
      y+=node[set[setNr].node[i]].ny*scale->w+scale->y;
      z+=node[set[setNr].node[i]].nz*scale->w+scale->z;
    }
    x/=(set[setNr].anz_p+set[setNr].anz_n);
    y/=(set[setNr].anz_p+set[setNr].anz_n);
    z/=(set[setNr].anz_p+set[setNr].anz_n);

    printf(" average location based on nodes and points: %e %e %e",x,y,z);
    sprintf(parameter[0],"%e",x);
    sprintf(parameter[1],"%e",y);
    sprintf(parameter[2],"%e",z);

    /* determine the average node-value */
    if(anz->l)
    {
      lfbuf=0.;
      for (i=0; i<set[setNr].anz_n; i++)
      {
        if(sequenceFlag) lfbuf+=lcase[lcase_animList].dat[animList][set[setNr].node[i]];
        else lfbuf+=lcase[cur_lc].dat[cur_entity][set[setNr].node[i]];
      }
      lfbuf/=set[setNr].anz_n;
      printf(" value: %e\n",lfbuf);
      sprintf(parameter[3],"%e",lfbuf);
      write2stack( 4, parameter);
    }
    else
    {
      printf("\n");
      write2stack( 3, parameter);
    }
  }
  else if (compareStrings(type, "BIA")>0)
  {
    pre_bia(&string[na+1]);
  }
  else if (compareStrings(type, "BODY")>0)
  {
    pre_body(&string[na+1]);
  }
  else if (compareStrings(type, "BREAK")>0)
  {
    printf("BREAK by user request\n");
    return(-1);
  }
  else if (compareStrings(type, "CALL")>0)
  {
    /* when node coordinates were changed to the deformed ones then switch back */ 
    if(addDispFlag)
    {
      addDispToCoordinates(node);
      // remember to switch back
      addDispFlagLocal=2;
    }
    if(vectorFlag) pre_view("vector off");
    descalAll();

    userFunction(&string[na+1], anz, anzGeo);

    descalAll();
    getScaleValues( setall, set, point, node, scale);
    scalPoints ( anzGeo->p, point, scale );
    scalNodes ( anz->n, node, scale);
    scalSurfs( anzGeo->s, surf, scale);
    // recalculate the line-shapes
    for (i=0; i<anzGeo->l; i++) repLine(i);
    // recalculate the nurbl-controll-points
    for (i=0; i<anzGeo->nurl; i++) repNurl(i);
    // recalculate the nurbs-controll-points
    for (i=0; i<anzGeo->nurs; i++) repNurs(i);
    //orientSet( "all" );

    if(anz->e>0)
    {
      adjustDrawNodes(0);
      makeSurfaces();
      getElemNormalen( e_enqire, node, anz->e );
      updateDispLists();
    }
    /* when node coordinates were changed to the deformed ones then switch back before they are copied and then switch again */ 
    if(addDispFlagLocal==2)
    {
      addDispToCoordinates(node);
    }
    if(activWindow!=-1) createDatasetEntries();
  }
  else if (compareStrings(type, "CAPT")>0)
  {
    stos( &string[na], 1, strlen(&string[na]), picture_caption);
    if(inpformat)
    {
      glutSetWindow( w0);
      glutPostRedisplay();
    }
  }
  else if (compareStrings(type, "CNTR")>0)
  {
    length=sscanf(&string[na+1], "%s %s %s", dat[0], dat[1], dat[2]); 
    if (length < 1) 
    { printf(" ERROR in formulation\n"); goto checkForError; }
    else if (length == 3)
    {
      center( (atof(dat[0])-scale->x)/scale->w,(atof(dat[1])-scale->y)/scale->w,(atof(dat[2])-scale->z)/scale->w);
    } 
    else
    {
      nr=getSetNr(dat[0]);
      if(nr<0)
      {
        nr=getPntNr(dat[0]);
        if(nr<0)
        {
          if(checkIfNumber(dat[0])==0)
          {
            printf(" ERROR: arg:%s is no set, point or node\n", dat[0]);
            goto checkForError;
          }
          nr=atoi(dat[0]);
          center( node[nr].nx, node[nr].ny, node[nr].nz);
        }
        else  center( point[nr].px, point[nr].py, point[nr].pz);
      }
      else
      {
        delSet(specialset->tmp);
        setNr=pre_seta(specialset->tmp, "i", 0 );
        seta( setNr, "se", nr );
        completeSet_frame(setNr, 0);
        getScaleValues( setNr, set, point, node, scaleSet);
        center( scaleSet->x, scaleSet->y, scaleSet->z);
        delSet(specialset->tmp);
      }
    }
  } 
  else if (compareStrings(type, "CONT")>0)
  {
    if(stopped_commandFile>-1)
    {
	if((commandFile[stopped_commandFile].handle!=NULL)&&(commandFile[stopped_commandFile].stopped))
        {
          printf("CONTINUE to read file:%s\n", commandFile[stopped_commandFile].name);
          readfbd( 0, 0);
          return(0);
	}
    }
    else printf("ERROR: No open commandFile\n");
  }
  else if (compareStrings(type, "COL")>0)
  {
    length=sscanf(&string[na+1], "%s", dat[0]);
    for(i=0; i<entitycols; i++)
    {
      if(compareStrings(dat[0],entitycol[i].name)>0)
      {
        entitycol[i].r=entitycol[i].g=entitycol[i].b=0;
        length=sscanf(&string[na+1], "%s %f %f %f", entitycol[i].name, &entitycol[i].r, &entitycol[i].g, &entitycol[i].b );
	goto checkForError;
      }
    }
    entitycols++;
    if( (entitycol= (Entitycol *)realloc(entitycol, entitycols*sizeof(Entitycol))) == NULL )
      printf("\n\n ERROR: malloc failed\n\n") ;
    i=entitycols-1;
    entitycol[i].r=entitycol[i].g=entitycol[i].b=0;
    length=sscanf(&string[na+1], "%s %f %f %f", entitycol[i].name, &entitycol[i].r, &entitycol[i].g, &entitycol[i].b ); 
  }
  else if (compareStrings(type, "COPY")>0)
  {
    pre_copy(&string[na+1]);
    redraw();
  }
  else if (compareStrings(type, "COMP")>0)
  {
    if ( sscanf(&string[na+1], "%s%s", name, xbuf) < 2) 
    { printf(" ERROR in formulation\n"); goto checkForError; }

    setNr=getSetNr(name);
    if (setNr<0)
    {
      /* set was not found. check if wildcards (*) were used */
      if((strlen(name)==1)&&(name[0]=='*'))
      {
        for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
        {
          if (!set[setNr].type)
          {
            printf(" comp %s %s\n", set[setNr].name, xbuf) ;
            completeSet( set[setNr].name, xbuf) ;
          }
        }
        goto checkForError;
      }
      length= strsplt( name, '*', &buf);
      if (length>0)
      {
        j=0;
        for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
        {
          foundSubString=0;
          for(i=0; i<length; i++)
          {
            if(strstr(set[setNr].name, buf[i]) !=NULL)
            {
              foundSubString++;
              /* check if the first or the last char is no '*' then the buf[] must be at start or end */
              if(i==0) { if(name[0]!='*')  { if(name[0]!=set[setNr].name[0])  foundSubString--; }  }
              if(i==length-1) { if(name[strlen(name)-1]!='*') { if(name[strlen(name)-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
	    }
	  }
          if(foundSubString==length)
	  {
            i=setNr;
            if (!set[i].type)
            {
              j++;
              printf(" comp %s %s\n", set[i].name, xbuf) ;
              completeSet( set[i].name, xbuf) ;
            }
	  }
	}
        if(j!=0) goto checkForError;
      }
      /* free buf */
      for(i=0; i<length; i++) free(buf[i]);
      free(buf);

      printf (" WARNING(comp): set '%s' does not exist\n", name);
      goto checkForError;
    }
    else  completeSet( name, xbuf) ;
  }
  else if (compareStrings(type, "CORRAD")>0)
  {
    length=sscanf(string, "%*s %s %lf",name,&x); 
    if (length==1) x=0.;
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" corrad: set:%s does not exist\n", name);
      goto checkForError;
    }
    corrad( setNr, x);
  }
  else if (compareStrings(type, "CSYSA")>0)
  {
    length = sscanf(&string[na+1], "%d %s", &nr, name);
    if(length<2) { printf(" wrong syntax\n"); goto checkForError; }
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" ERROR in csysa: set:%s does not exist\n", name);
      goto checkForError;
    }
    nodeCsysSet[nodeCsys]=setNr;
    nodeCsysNr[nodeCsys]=nr;
    nodeCsys++;
  }
  else if (compareStrings(type, "CUT")>0)
  {
    ibuf=sscanf(&string[na+1], "%s %s %s", dat[0], dat[1], dat[2]);
    if(ibuf==1)
    {
      // check if it's a single node
      nr=atoi(dat[0]);
      if(nr<1)
      {
	// check if it's a set
	nr=getSetNr(dat[0]);
        if(nr<1)
        {
          printf(" ERROR: arg:%s is not a node nor a set\n", dat[i]);
          goto checkForError;
	}
        for (i=0; i<set[nr].anz_p; i++)
        {
	  if(i>2) break;
	  strcpy(dat[i],point[set[nr].pnt[i]].name);
	}
        for (j=0; j<set[nr].anz_n; j++)
        {
	  if(i>2) break;
	  sprintf(dat[i],"%d",set[nr].node[j]);
	  i++;
	}
	ibuf=i;
        if(ibuf==1) { nr=getSetNr(dat[0]); pre_cut( nr, 'v' ); goto checkForError; }
      }
      else { pre_cut( nr, 'v' ); goto checkForError; }
    }
    if(ibuf==3)
    {
      for (i=0; i<3; i++)
      {
        nr=getPntNr(dat[i]);
	if(!addFlag)
  	{
	  operateAlias( dat[i], "p" );
	  nr=getPntNr(dat[i]);
	}
	else
	{
          name[0]='%';
	  strcpy(&name[1],dat[i]);
	  operateAlias( name, "p" );
	  nr=getPntNr(name);
	}
	if(nr<0)
        {
          if(checkIfNumber(dat[i])==0)
	  {
            printf(" ERROR: arg:%s is no point and no node\n", dat[i]);
            goto checkForError;
          }
          nr=atoi(dat[i]);
          pre_cut( nr, 'n' );
        }  
        else pre_cut( nr, 'p' );
      }
    }
    else
    {
      printf(" ERROR: wrong number of arguments. One node or 3 points or nodes needed\n");
      goto checkForError;
    }
    goto checkForError;
  }
  else if (compareStrings(type, "DEL")>0)
  {
    pre_del(&string[na+1]);
  }
  else if (compareStrings(type, "DIST")>0)
  {
    na=sscanf(string, "%*s%s%s",dat[0],dat[1]);
    if(na==1)
    {
      setNr=getSetNr(dat[0]);
      x=y=z=-MAX_FLOAT;
      dx=dy=dz=MAX_FLOAT;
      if(setNr>-1)
      {
        for( i=0; i<set[setNr].anz_n; i++ )
	{
          j=set[setNr].node[i];
          x = dmax(x,node[j].nx);
          dx = dmin(dx,node[j].nx);
          y = dmax(y,node[j].ny);
          dy = dmin(dy,node[j].ny);
          z = dmax(z,node[j].nz);
          dz = dmin(dz,node[j].nz);
	}
        for( i=0; i<set[setNr].anz_p; i++ )
	{
          j=set[setNr].pnt[i];
          if( point[j].name != (char *)NULL )
	  {
            x = dmax(x,point[j].px);
            dx = dmin(dx,point[j].px);
            y = dmax(y,point[j].py);
            dy = dmin(dy,point[j].py);
            z = dmax(z,point[j].pz);
            dz = dmin(dz,point[j].pz);
	  }
        }
      }
      dx=(x-dx)* scale->w;
      dy=(y-dy)* scale->w;
      dz=(z-dz)* scale->w;
      printf(" dxyz: %e %e %e\n", dx,dy,dz);
      sprintf(parameter[0],"%e",dx);
      sprintf(parameter[1],"%e",dy);
      sprintf(parameter[2],"%e",dz);
      write2stack( 3, parameter);
    }
    else pre_proj( string );
  }
  else if (compareStrings(type, "DIV")>0)
  {
    /*
    descalAll();
    getScaleValues(0, set, point, node, scale);
    scalNodes ( anz->n, node, scale );
    scalPoints ( anzGeo->p, point, scale );
    scalSurfs( anzGeo->s, surf, scale);
    // recalculate the line-shapes
    for (i=0; i<anzGeo->l; i++) repLine(i);
    // recalculate the nurbl-controll-points
    for (i=0; i<anzGeo->nurl; i++) repNurl(i);
    // recalculate the nurbs-controll-points
    for (i=0; i<anzGeo->nurs; i++) repNurs(i);
    // correct the orientation of all entities
    orientSet( "all" );
    */
    pre_div(&string[na+1]);
  }
  else if ((compareStrings(type, "DS")>0)||(compareStrings(type, "LC")>0))
  {
    sscanf(string,"%*s %s",buffer);
    if(buffer[0]=='e')
    {
      strcpy(name,"user");
      ncomps=1;
      analysis_type=0;
      i=0;
      j=0;
      sscanf(string,"%*s %*s %s %d %d %d %d", name, &ncomps, &analysis_type, &i, &j);
      if(i==0) i=ncomps;
      if(analysis_type==0) analysis_type=1;
      if(defineEntity(anz, lcase, node, name, ncomps, analysis_type, i, j)!=0) printf(" WARNING: entity:%d of dataset %d could not be set.\n", ncomps, cur_lc+1);
      if(activWindow!=-1) createDatasetEntries();
    }
    else if(buffer[0]=='g')
    {
      ncomps=0;
      sscanf(string,"%*s %*s %*s %d", &ncomps);
      if(!ncomps)
      {
        if(!anz->l) { printf(" ERROR: No current dataset exists.\n"); return(-1); }
	lfbuf=lcase[cur_lc].value;
	strcpy( name, lcase[cur_lc].name) ;
	strcpy(dataset_text, lcase[cur_lc].dataset_text);
	strcpy(analysis_name, lcase[cur_lc].analysis_name);
        step_number=lcase[cur_lc].step_number;
        analysis_type=lcase[cur_lc].analysis_type;
      }
      /* defaults */
      else
      {
        strcpy(name,"unnamed");
        lfbuf=0.;
        dataset_text[0]=0;
        analysis_type=1;
        if(anz->l) step_number=lcase[anz->l-1].step_number+1; else step_number=1;
        analysis_name[0]=0;
      }
      sscanf(string,"%*s %*s %s %d %lf %s %d %d %s", name, &ncomps, &lfbuf, dataset_text, &analysis_type, &step_number, analysis_name);
      generateDataset(anz, &lcase, name, ncomps, lfbuf, dataset_text, analysis_type, step_number, analysis_name);
      recompileEntitiesInMenu(anz->l-1);
      if(activWindow!=-1) createDatasetEntries();
    }
    else if(buffer[0]=='f')
    {
        for(n=0; n<anz->n; n++)
        {
          for(i=0; i<lcase[cur_lc].ncomps; i++)
          {
            if (lcase[cur_lc].dat[i][node[n].nr] >  lcase[cur_lc].max[i])
            {  lcase[cur_lc].max[i]=lcase[cur_lc].dat[i][node[n].nr]; lcase[cur_lc].nmax[i]=node[n].nr; }
            if (lcase[cur_lc].dat[i][node[n].nr] <  lcase[cur_lc].min[i])
            {  lcase[cur_lc].min[i]=lcase[cur_lc].dat[i][node[n].nr]; lcase[cur_lc].nmin[i]=node[n].nr; }
          }
        }
      calcDatasets( anz->l-1, anz, node, lcase );
      if(backgroundFlag) {  backgroundFlag=0; inpformat=inpformatbuffer; }
      createDatasetEntries();
    }
    else if(buffer[0]=='l') { selectData( &string[na+1] ); }
    else if(checkIfNumber(buffer)) { selectData( &string[na+1] ); }
    else { printf(" ERROR: Parameter:%s not recognized.\n", buffer); }
  }
  else if (compareStrings(type, "ELEM")>0)
  {
    dat[0][0]=0;
    sscanf(string,"%*s %s %s",dat[0], dat[1]);
    if(dat[0][0]=='f')
    {
      setNr=getSetNr(dat[1]);
      printf(" changed elements:%d\n",elemChecker( set[setNr].anz_e, set[setNr].elem, node, e_enqire));
    }
    else
    pre_elem(&string[na+1]);
  }
  else if (compareStrings(type, "ELTY")>0)
  {
    pre_elty(&string[na+1]);
  }
  else if (compareStrings(type, "ENQ")>0)
  {
    enquireEntities(&string[na+1]);
  }
  else if (compareStrings(type, "EPROP")>0)
  {
    pre_eprop(&string[na+1]);
  }
  else if (compareStrings(type, "EQAL")>0)
  {
    pre_eqal(&string[na+1]);
  }
  else if ( compareStrings(type, "EXIT")>0 )
  {
    strcpy(setname,datin);
    for (i=strlen(datin); i>=0; i--)
    {
      if(datin[i]=='.')break;
    }
    setname[i]='\0';
#ifdef WIN32
    sprintf (buffer, "move /y \"%s\" \"%s.fbb\"", datin, setname );
#else
    sprintf (buffer, "mv %s %s.fbb", datin, setname );
#endif
    if(compare(&datin[i+1],"fbd",3)==3) system (buffer);
    pre_write(" all fbd ");
    if( compareStrings( setname, "all")<3)
    {
#ifdef WIN32
      sprintf (buffer, "move /y all.fbd \"%s.fbd\"", setname );
#else
      sprintf (buffer, "mv all.fbd %s.fbd", setname );
#endif
      system (buffer);
    }     
    exit(0);
  }
  else if (compareStrings(type, "FIL")>0)
  {
    /* intersection of 2 lines */ 
    if (!addFlag)    length = sscanf(string, "%*s%s%s%lf", dat[0], dat[1], &x);
    else 
    {
      name[0]='!';
      dat[0][0]=dat[1][0]='%';
      length = sscanf(string, "%*s%s%s%lf", &dat[0][1],  &dat[1][1], &x);
    }
    operateAlias( dat[0], "l" );
    operateAlias( dat[1], "l" );
    i=getLineNr(dat[0]);
    j=getLineNr(dat[1]);
    x/=scale->w;

    pscal=gtol;
    gtol=MAX_FLOAT;
    createFillet(i, x);
    createFillet(j, x);
    gtol=pscal;
    repLine(i);
    repLine(j);
  }
  else if (compareStrings(type, "FLIP")>0)
  {
    na=sscanf(string, "%*s%s%s",dat[0],dat[1]);
    nr=getSetNr(dat[0]);
    if((nr>-1)&&(set[nr].type==1)) nr=-1;
    if((nr>-1)&&(set[nr].anz_s==1)) { strcpy(dat[0],surf[set[nr].surf[0]].name); nr=-1; }
    if(nr>-1)
    {
      if(dat[1][0]=='e') for(i=0; i<set[nr].anz_e; i++) flip("e",set[nr].elem[i]);
      else if(dat[1][0]=='s') for(i=0; i<set[nr].anz_s; i++) flip("s",set[nr].surf[i]);
      else if(dat[1][0]=='b') for(i=0; i<set[nr].anz_b; i++) flip("b",set[nr].body[i]);
      else
      {
        if(!set[nr].anz_s) for(i=0; i<set[nr].anz_e; i++) flip("e",set[nr].elem[i]);
        for(i=0; i<set[nr].anz_s; i++) flip("s",set[nr].surf[i]);
        for(i=0; i<set[nr].anz_b; i++) flip("b",set[nr].body[i]);
      }
    }
    else
    {
      nr=getSurfNr(dat[0]);
      if(nr>-1)
      {
        if((na>1)&&(dat[1][0]=='a'))  flip("sa",nr);
        else flip("s",nr);
        goto checkForError;
      }
      nr=getBodyNr(dat[0]);
      if(nr>-1)
      {
        flip("b",nr);  
        goto checkForError;
      }
      nr=atoi(dat[0]);
      if(nr>0)
      {
        flip("e",nr);  
        goto checkForError; 
      }
      printf(" ERROR Name unknown:%s\n",dat[0]);
    }
  }
  else if (compareStrings(type, "FLPC")>0)
  {
    flipColorFlag=!flipColorFlag;
    redraw();
  }
  else if (compareStrings(type, "FONT")>0)
  {
    i=DEF_GLUT_FONT+1;
    sscanf(&string[na+1],"%s %d",buffer,&i);
    if((i>0)&&(i<=SUM_GLUT_FONTS))
    {
      i--;
      if(compare(buffer, "legend", 1)==1) legend_font=i;
      else if(compare(buffer, "draw", 1)==1) draw_font=i;
      else if(compare(buffer, "menu", 1)==1) menu_font=i;
      else if(compare(buffer, "cl", 1)==1) menu_font=i;
      else printf(" WARNING: Unknown target:%s\n", buffer);
      redraw();
      reshape( width_w0, height_w0 );
    }
    else printf(" WARNING: Unknown font index:%d\n",i);
  }
  else if (compareStrings(type, "FRAME")>0)
  {
    na=sscanf(string, "%*s%s",name);
    if(na>0)
    {
      setNr=getSetNr(name);
      if (setNr<0)
      {
        printf (" set:%s does not exist\n", name);
        goto checkForError;
      }
      frameSetFlag=setNr;
    }
    else frameSetFlag=-1; 
    frame();
  }
  else if (compareStrings(type, "GBOD")>0)
  {
    pre_gbod(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "GONLY")>0)
  {
    if ( compare(&string[na+1], "of", 2) == 2)
    { 
      frameFlag=1;
      captionFlag=1;
      textFlag=1;
      scalaFlag=1; 
    }
    else
    { 
      frameFlag=0;
      captionFlag=0;
      textFlag=0;
      scalaFlag=0;
    }
    redraw();
    if(inpformat)
    {
      glutSetWindow( w0);
      glutPostRedisplay();
    }
  }
  else if (compareStrings(type, "GRAPH")>0)
  {
    nr=graph(&string[na+1]);

    /* actual graph nr */
    sprintf(buffer,"!graph_Nr %d", nr);
    pre_value(buffer);
  }
  else if (compareStrings(type, "GRPA")>0)
  {
    length = sscanf(&string[na+1], "%d %s", &nr, name);
    if(length<2) { printf(" wrong syntax\n"); goto checkForError; }
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" ERROR in mata: set:%s does not exist\n", name);
      goto checkForError;
    }
    for (i=0; i<anz->e; i++)
    {
      e_enqire[e_enqire[i].nr].group=nr;
    }
  }
  else if (compareStrings(type, "GRPS")>0)
  {
    for (i=0; i<anz->e; i++)
    {
      sprintf(name,"%d", e_enqire[i].nr);
      sprintf(buffer, "+grp%d", e_enqire[e_enqire[i].nr].group );
      pre_seta( buffer, "e", name  );
    }
    prnt("se +grp*");
  }
  else if (compareStrings(type, "TYPS")>0)
  {
    for (i=0; i<anz->e; i++)
    {
      sprintf(name,"%d", e_enqire[i].nr);
      sprintf(buffer, "+typ%d", e_enqire[e_enqire[i].nr].type );
      pre_seta( buffer, "e", name  );
    }
    prnt("se +typ*");
  }
  else if (compareStrings(type, "GROUP")>0)
  {
    printf("ERROR: Not longer supported.\n");
  }
  else if (compareStrings(type, "GSUR")>0)
  {
    pre_gsur(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "GTOL")>0)
  {
    if(gtolFlag>(int *)NULL) *gtolFlag=1;
    length=sscanf(string, "%*s %s",buffer); 
    if (length==1)
    {
      if(buffer[0]=='a') { gtol=calcGTOL(setall);  printf ("gtol new calculated:%e\n", gtol); }
      else { gtol=atof(buffer); printf ("gtol set to:%e\n", gtol); }
    }
    else printf ("gtol:%e\n", gtol);

    if(valuestackFlag)
    {
      n=1;
      if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
      for(i=0; i<n; i++)
      {
        if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
        { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
      }
      sprintf(valuestack[valuestack_ptr++],"%e", gtol );
      printf(" gtol written to stack\n");
    }
  }
  else if (compareStrings(type, "HCPY")>0)
  {
    format[0]=0;
    args=sscanf(string,"%*s %s %s", name, format);
    if(args==2) sbuf=format; else sbuf=NULL;
    if     (compareStrings(name, "ps")>0)  createHardcopy(1,sbuf);
    else if(compareStrings(name, "tga")>0) createHardcopy(2,sbuf);
    else if(compareStrings(name, "gif")>0) createHardcopy(4,sbuf);
    else if(compareStrings(name, "png")>0) createHardcopy(5,sbuf);
    else if(compareStrings(name, "clean")>0)
    {
      sprintf( buffer, "rm -f hcpy_*.* %s", DEV_NULL2);
      system (buffer);
      psNr=tgaNr=gifNr=pngNr=0;
    }
    else if(compareStrings(name, "make")>0)
    {
      /* open the actual dir and get the available hcpy files */
      pics=0;
      dirp = opendir(".");
      if (dirp != NULL) while ((dp = readdir(dirp)) != NULL)
      {
        /* search for hcpy_xx.ps files  */
        if( (compare(dp->d_name, "hcpy", 4) == 4) && ( dp->d_name[strlen(dp->d_name)-2]=='p') && ( dp->d_name[strlen(dp->d_name)-1]=='s') )
        {
          printf(" found %s \n", dp->d_name);
          /* get the pic-nr */
          i=0; while(dp->d_name[i]!='_') i++; i++;
          j=0; while(dp->d_name[i]!='.') { posn[j]=dp->d_name[i]; j++; i++;}
          posn[j]=0;
          if ( (picnr = (int *)realloc( (int *)picnr, (pics+2) * sizeof(int))) == NULL )
          {  printf("\n\n ERROR: realloc failed, hcpy\n\n"); }
          picnr[pics]=atoi(posn); pics++;
        }
      }
      closedir(dirp);
      qsort( picnr, pics, sizeof(int), (void *)compareInt );

      nr=0; /* page-nr */
      ibuf=0; /* pic-nr */
      do
      {
        sprintf( buffer, "gs -dNOPAUSE -dBATCH -sDEVICE=ps2write -sOutputFile=tmp.ps ");
        for(j=0; j<6; j++) /* 6 pics per page */
	{
          if(ibuf==pics) break;
          sprintf( &buffer[strlen(buffer)], " hcpy_%d.ps", picnr[ibuf++]);
	}
        system (buffer);
        printf("%s\n",buffer);
        if ( compareStrings(format, "ls")>0)
        {
          pscal=0.42;
          x=11.; dx=9.;
          y=1.5;  dy=9.;
          /* sscanf(string,"%*s %*s %*s %lf %lf %lf %lf %lf", &x, &y, &dx, &dy, &pscal); */
          
          // activate this lined if ps2write is used
          //sprintf( buffer, "convert -density %dx%d tmp.ps tmp.ps", (int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)),(int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)));
          system (buffer);
          printf("%s\n",buffer);

          sprintf( buffer, "pstops '6:0L@%lf(%lfcm,%lfcm)+3L@%lf(%lfcm,%lfcm)+1L@%lf(%lfcm,%lfcm)+4L@%lf(%lfcm,%lfcm)+2L@%lf(%lfcm,%lfcm)+5L@%lf(%lfcm,%lfcm)' tmp.ps tmp_%d.ps", pscal, x,y, pscal, x+dx,y, pscal, x,y+dy, pscal, x+dx,y+dy, pscal, x,y+2.*dy, pscal, x+dx,y+2.*dy, nr);
        }
        else
        {
          pscal=0.42;
          x=2.; dx=9.;
          y=1.; dy=9.;
          /* sscanf(string,"%*s %*s %*s %lf %lf %lf %lf %lf", &x, &y, &dx, &dy, &pscal); */
          
          // activate this lined if ps2write is used
          //sprintf( buffer, "convert -density %dx%d tmp.ps tmp.ps", (int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)),(int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)));
          system (buffer);
          printf("%s\n",buffer);

          sprintf( buffer, "pstops '6:4@%lf(%lfcm,%lfcm)+5@%lf(%lfcm,%lfcm)+2@%lf(%lfcm,%lfcm)+3@%lf(%lfcm,%lfcm)+0@%lf(%lfcm,%lfcm)+1@%lf(%lfcm,%lfcm)' tmp.ps tmp_%d.ps", pscal, x,y, pscal, x+dx,y, pscal, x,y+dy, pscal, x+dx,y+dy, pscal, x,y+2.*dy, pscal, x+dx,y+2.*dy, nr);
        }
        system (buffer);
        printf("%s\n",buffer);
        sprintf( buffer, "gs -dNOPAUSE -dBATCH -sDEVICE=ps2write -sOutputFile=cgx_%d.ps tmp_%d.ps", nr, nr);
        system (buffer);
        printf("%s\n",buffer);
        sprintf( buffer, "rm -f tmp*.ps %s",DEV_NULL);
        system (buffer);
        nr++;
      }while(j==6);
      sprintf( buffer, "gs -dNOPAUSE -dBATCH -sDEVICE=ps2write -sOutputFile=cgx.ps ");
      for(j=0; j<nr; j++)
      {
        sprintf( &buffer[strlen(buffer)], " cgx_%d.ps", j);
      }
      system (buffer);
      printf("%s\n",buffer);
      sprintf( buffer, "rm -f cgx_*.ps %s",DEV_NULL);
      system (buffer);
      // TBD: if ( compareStrings(format, "ls")>0)
      if(inpformat) sprintf(buffer, "%s cgx.ps &", psviewer);
      system (buffer);
      printf("ready\n");

      // activate this lined if ps2write is used
      printf(" you may convert to pdf with 'convert -density 600x600 cgx.ps cgx.pdf\n");

    }
    else if(compareStrings(&string[na+1], "clean")>0)
    {
      sprintf( buffer, "rm -f hcpy_* %s",DEV_NULL);
      system (buffer);
      psNr=tgaNr=gifNr=pngNr=0;
    }           
    else createHardcopy(2, NULL);
  }
  else if (compareStrings(type, "INT")>0)
  {
    /* intersection of 2 lines */ 
    if (!addFlag)    length = sscanf(string, "%*s%s%s", dat[0], dat[1]);
    else 
    {
      name[0]='!';
      dat[0][0]=dat[1][0]='%';
      length = sscanf(string, "%*s%s%s", &dat[0][1],  &dat[1][1]);
    }
    operateAlias( dat[0], "l" );
    operateAlias( dat[1], "l" );
    i=getLineNr(dat[0]);
    j=getLineNr(dat[1]);

    pscal=gtol;
    gtol=MAX_FLOAT;
    intersect(i);
    intersect(j);
    gtol=pscal;
    repLine(i);
    repLine(j);
  }
  else if (compareStrings(type, "LCMB")>0)
  {
    pre_lcmb(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "LENGTH")>0)
  {
    pre_length(&string[na+1]);
  }
  else if (compareStrings(type, "LINE")>0)
  {
    pre_line(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "LNOR")>0)
  {
    if (!addFlag)    length = sscanf(string, "%*s%s%s%s%s%lf", name, dat[1], dat[2], dat[0], &pscal);
    else 
    {
      name[0]='!';
      dat[0][0]=dat[1][0]=dat[2][0]='%';
      length = sscanf(string, "%*s%s%s%s%s%lf", &name[1],  &dat[1][1],  &dat[2][1],  &dat[0][1], &pscal);
    }
  
    /* check if an lcmb uses this name, not use it */
    if (getLcmbNr(name) >0) {errMsg("ERROR: LCMB uses this name %s, try another name \n", name); return(-1);}
    if(length==5)
    {
      operateAlias( name, "l" );
      operateAlias( dat[0], "p" );
      operateAlias( dat[1], "p" );
      operateAlias( dat[2], "p" );
    }
    else if(length==3)
    {
      pscal=atof(dat[2]);
      operateAlias( dat[1], "se" );
      // check if it's a set
      nr=getSetNr(dat[1]);
      if(nr<1)
      {
        printf(" ERROR: arg:%s is not a set\n", dat[1]);
        goto checkForError;
      }
      for (i=0; i<set[nr].anz_p; i++)
      {
        if(i>2) break;
        strcpy(dat[i],point[set[nr].pnt[i]].name);
      }
    }     
    if (!pscal) {errMsg("ERROR: length must not be 0\n"); return(-1);}

    for(i=0; i<3; i++) pnts[i]=getPntNr(dat[i]);
    if((i=normalLine(name, pnts, pscal))<0)
    { printf("lnor: could not create new line\n");  return(-1);}
    else { printf(" create line:%s with new end-point %s\n", name, point[i].name ); }
  }
  else if (compareStrings(type, "INIT")>0)
  {
    initModel(string);
    if(backgrndcol) sprintf(buffer,"bg w"); else sprintf(buffer,"bg k");
    pre_view(buffer); 
    if (cullFlag) sprintf(buffer,"back"); else sprintf(buffer,"front");
    pre_view(buffer);
    redraw();
  }
  else if (compareStrings(type, "MAP")>0)
  {
    dat[0][0]=0;
    length = sscanf(string, "%*s%s%s%s%s%s", setname, setname2, name, dataset, dat[0]);
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" set:%s does not exist\n", setname);
      goto checkForError;
    }
    setNr2=getSetNr(setname2);
    if (setNr2<0)
    {
      printf (" set:%s does not exist\n", setname2);
      goto checkForError;
    }
    if (set[setNr2].anz_n<=0)
    {
      printf (" set:%s does not contain nodes\n", setname2);
      goto checkForError;
    }

    /* mapping needs stronger gtol */
    /* WARNING: mapping of 'nan' values might lead to segfault! */
    x=gtol;
    gtol*=GTOL_FACTOR_MAP;

    if ((compare(name, "surf", 1)==1)||(name[0]== 'r')||(name[0]== 'x')||(name[0]== 'y')||(name[0]== 'z'))
    {
      if(dat[0][0]=='n') areampc(setNr, setNr2, "map", name, dataset, 0, 0, node, 0, 1);
      else areampc(setNr, setNr2, "map", name, dataset, 0, 0, node, 1, 1);
    }
    else if (compare(name, "volu", 1)==1)
    {
      if(dat[0][0]=='n') i=interpol3d(setNr, setNr2, name, dataset, 0);
      else i=interpol3d(setNr, setNr2, name, dataset, 1 );
      if(i) printf(" WARNING: set %s includes nodes which could not be mapped\n", set[i].name);
    }
    else
    {
      printf(" ERROR: Neither 'surf' nor 'volu' given: %s\n", name);
      goto checkForError;
    }
    
    printf(" Mapping done with %e times gtol=%e. The user might increase gtol if needed.\n",GTOL_FACTOR_MAP,gtol);
    gtol=x;

    redraw();
  }
  else if (compareStrings(type, "MATA")>0)
  {
    length = sscanf(&string[na+1], "%d %s", &nr, name);
    if(length<2) { printf(" wrong syntax\n"); goto checkForError; }
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" ERROR in mata: set:%s does not exist\n", name);
      goto checkForError;
    }
    mata( elemMat, nr, setNr);
  }
  else if (compareStrings(type, "MATS")>0)
  {
    for (i=0; i<anz->e; i++)
    {
      sprintf(name,"%d", e_enqire[i].nr);
      sprintf(buffer, "+mat%d", e_enqire[e_enqire[i].nr].mat );
      pre_seta( buffer, "e", name );
    }
    prnt("se +mat*");
  }
  else if (compareStrings(type, "MAXC")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smax,&scale->format,&scale->lock );
    scale->smaxr=2;
    redraw();
  }
  else if (compareStrings(type, "MAXR")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smax,&scale->format,&scale->lock );
    scale->smaxr=1;
    redraw();
  }
  else if (compareStrings(type, "MAX")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smax,&scale->format,&scale->lock );
    scale->smaxr=0;
    redraw();
  }
  else if (compareStrings(type, "MENU")>0)
  {
    pre_menu( &string[na+1] );
  }
  else if (compareStrings(type, "MERG")>0)
  {
    pre_merge( &string[na+1] );
    redraw();
  }
  else if (compareStrings(type, "MESH")>0)
  {
    if(addDispFlag)
    {
      addDispToCoordinates(node);
      addDispFlagLocal=2;
    }
    delSet("-EDGE");
    nr=pre_mesh( &string[na+1] );
    //adjustDrawNodes(0);
    //getElemNormalen( e_enqire, node, anz->e );
    //makeSurfaces();
    if(automode==1) { fixBadDivisions(nr); automode=0; }

    if(addDispFlagLocal==2)
    {
      addDispToCoordinates(node);
    }
    redraw();
  }
  else if (compareStrings(type, "MIDS")>0)
  {
    if(addDispFlag==1)
    {
      printf (" WARNING: displacements were added to the node-coords. This is not permitted\n");
      addDispToCoordinates(node);
    }
    pre_view(" surf");
    sscanf( string, "%*s %s %s", setname, comm);

    /* free the additional midside-nodes for higher order elements */
    for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
    anz->n= anz->orign;
    anz->nmax=anz->orignmax;
    fixMidsideNodes( setname, comm );

    adjustDrawNodes(1);

    makeSurfaces();        // includes getFaceNormalen 
    getElemNormalen( e_enqire, node, anz->e );
    if(compare(comm,"gen",2)==2)
    {
      for (j=0; j<anz->l; j++)
      {
        freeDataset(lcase, j);
      }
      free(lcase); lcase=NULL;
      anz->l=0;
      printf (" WARNING: Results were intentionally not interpolated to new nodes. Do not work with Datasets any more.\n");
      updateDispLists();
      createNewMainMenu();
    }
    else
    {
      updateDispLists();
    }
    redraw();
  }
  else if (compareStrings(type, "MINUS")>0)
  {
    minus(&string[na+1] );
  }
  else if (compareStrings(type, "MINC")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smin,&scale->format,&scale->lock );
    scale->sminr=2;
    redraw();
  }
  else if (compareStrings(type, "MINR")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smin,&scale->format,&scale->lock );
    scale->sminr=1;
    redraw();
  }
  else if (compareStrings(type, "MIN")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smin,&scale->format,&scale->lock );
    scale->sminr=0;
    redraw();
  }
  else if (compareStrings(type, "MM")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); goto checkForError; }
    sscanf( string, "%*s %lf %c %c",&scale->smax,&scale->format,&scale->lock );
    scale->smin=-scale->smax;
    redraw();
  }
  else if (compareStrings(type, "MOVE")>0)
  {
    pre_move( &string[na+1] );
    redraw();
  }
  else if (compareStrings(type, "MOVI")>0)
  {
    pre_movie( &string[na+1] );
  }
  else if (compareStrings(type, "MSHP")>0)
  {
    buffer[0]=0;
    k=0;
    l=0;
    n=0;
    if(!addFlag) length = sscanf( string, "%*s%s%s%d%d%d%s", name, entity, &k,&l,&n, buffer);
    else
    {
      name[0]='%';
      length = sscanf( string, "%*s%s%s%d%d%d%s", &name[1], entity, &k,&l,&n, buffer);
    }
    if(entity[0]=='l')
    {
      operateAlias( name,"l");
      nr=getLineNr(name);
      if(nr>-1)
      {
        line[nr].etyp=k;
        line[nr].eattr=l;
        line[nr].elock=n;
      }
    }
    else if(entity[0]=='s')
    {
      operateAlias( name,"s");
      nr=getSurfNr(name);
      if(nr>-1)
      {
        if(strlen(buffer))
	{
          if((surf[nr].eparm= (char *)realloc((char *)surf[nr].eparm, (strlen(buffer)+1)*sizeof(char))) == NULL )
          { printf("ERROR: malloc failed\n\n" ); goto checkForError; }
          strcpy(surf[nr].eparm, buffer);
	}
        surf[nr].etyp=k;
        surf[nr].eattr=l;
        surf[nr].elock=n;
      }
    }
    else if(entity[0]=='b')
    {
      operateAlias( name,"b");
      nr=getBodyNr(name);
      if(nr>-1)
      {
        if(strlen(buffer))
	{
          if((body[nr].eparm= (char *)realloc((char *)body[nr].eparm, (strlen(buffer)+1)*sizeof(char))) == NULL )
          { printf("ERROR: malloc failed\n\n" ); goto checkForError; }
          strcpy(body[nr].eparm, buffer);
	}
        body[nr].etyp=k;
        body[nr].eattr=l;
        body[nr].elock=n;
      }
    }
  }
  else if ( compareStrings(type, "MSG")>0 )
  {
    sscanf( string, "%*s%s", buffer );
    if ( compareStrings(buffer, "on")>0) printFlag=1; 
    else if ( compareStrings(buffer, "off")>0) printFlag=0;
    else printf(" %s not recognized (use on/off)\n", buffer);
  }
  else if (compareStrings(type, "NEIGH")>0)
  {
    pre_contact(&string[na+1]);
  }
  else if (compareStrings(type, "NODE")>0)
  {
    if(addDispFlag==1) { printf (" WARNING: displacements were added to the node-coords. This is not permitted\n"); addDispToCoordinates(node); }
    sscanf(&string[na+1],"%d %s",&nr,buffer);
    if(buffer[0]!='v') pre_nod(&string[na+1]);
    else
    {
      if((nr<1)||(nr>anz->nmax))
      {
        printf (" WARNING: node:%d from string:%s not known\n", nr,&string[na+1]);
        returnFlag=-1;
	goto checkForError;
      }
      else if(anz->l)
      {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[cur_lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , cur_lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", cur_lc+1); 
            returnFlag=-1;
 	    goto checkForError;
          }
	}
        if(buffer[1]=='s') lcase[cur_lc].dat[cur_entity][nr]=atof(dat[i+2]);
        else
	{
          args=brecord(&string[na], dat);
          if( lcase[cur_lc].ncomps < (args-2) ) 
          {
            printf(" ERROR: Defined entities:%d but active dataset:%d has only:%d entities\n",args-2, cur_lc+1,lcase[cur_lc].ncomps );
            returnFlag=-1;
 	    goto checkForError;
          }
          for(i=0; i<args-2; i++) lcase[cur_lc].dat[i][nr]=atof(dat[i+2]);
	}
      }
      else
      {
        printf(" ERROR: No active dataset available\n");
        returnFlag=-1;
        goto checkForError;
      }
    }
  }
  else if (compareStrings(type, "NORM")>0)
  {
    pre_norm(&string[na+1]);
  }
  else if (compareStrings(type, "NURL")>0)
  {
    if(nurl( &string[na+1], addFlag )==-1) return(-1);
  }
  else if (compareStrings(type, "NURS")>0)
  {
    i=pre_nurs( &string[na+1], addFlag);
    if(i==-1) return(-1);
  }
  else if (compareStrings(type, "ORI")>0)
  {
    orientSet( &string[na+1] );
  }
  else if (compareStrings(type, "PLOT")>0)
  {
    /* trigger new scale-values */
    length=sscanf(&string[na+1],"%s", format);
    if((compareStrings(format,"ev")>0)||(compareStrings(format,"fv")>0)) { if(scale->lock!='l') scale->smin=scale->smax=0; }
    plot( &string[na+1] );
  }
  else if (compareStrings(type, "PLUS")>0)
  {
    /* trigger new scale-values */
    if((compareStrings(format,"ev")>0)||(compareStrings(format,"fv")>0)) { if(scale->lock!='l') scale->smin=scale->smax=0; }
    plus( &string[na+1] );
  }
  else if (compareStrings(type, "PNT")>0)
  {
    pre_pnt( &string[na+1], addFlag );
  }
  else if (compareStrings(type, "PROJ")>0)
  {
    if(addDispFlag==1) { printf (" WARNING: displacements were added to the node-coords. Projection of nodes might fail.\n"); }
    pre_proj( string );
  }
  else if (compareStrings(type, "PROC")>0)
  {
    pre_proc( &string[na+1] );
  }
  else if (compareStrings(type, "PRNT")>0) prnt(&string[na+1]);
  else if (compareStrings(type, "QUIT")>0) exit(0);
  else if (compareStrings(type, "READ")>0)
  {
    returnFlag=pre_read(&string[na+1]); 
  }
  else if (compareStrings(type, "REP")>0)
  {
    nr=getSetNr(&string[na+1]);
    if(nr<0)
    {
      printf("ERROR: Set:[%s] in command:|%s| does not exist\n",&string[na+1],string);
      goto checkForError;
    } 
    delSet(specialset->bsur);
    set_bsur=pre_seta(specialset->bsur, "i", 0);
    if( compareStrings(&string[na+1],"all")>0)
    {
      for (i=0; i<anzGeo->l; i++) repLine(i);
      for (i=0; i<anzGeo->nurl; i++) repNurl(i);
      for (i=0; i<anzGeo->nurs; i++) repNurs(i);
      /* delete the data-structures for trimming of the nurbs */
      for (i=0; i<anzGeo->nurs; i++) untrimNurs(i);
      /* recalculate the nurbs-trimming-points (after repNurs) */
      /* set "all" to avoid substitute surfs */
      j=getSetNr("all");
      //if(j>=0) for (i=0; i<set[j].anz_s; i++) repSurf(set[j].surf[i],1);
      pre_repSurf(j);
      for (i=0; i<anzGeo->nurs; i++) untrimNurs(i);
    }
    else
    {
      for (i=0; i<set[nr].anz_l; i++) repLine(set[nr].line[i]);
      for (i=0; i<set[nr].anz_nurl; i++) repNurl(set[nr].nurl[i]);
      for (i=0; i<set[nr].anz_nurs; i++) repNurs(set[nr].nurs[i]);
      for (i=0; i<set[nr].anz_nurs; i++) untrimNurs(set[nr].nurs[i]);
      //for (i=0; i<set[nr].anz_s; i++) repSurf(set[nr].surf[i],1);
      pre_repSurf(nr);
      for (i=0; i<set[nr].anz_nurs; i++) untrimNurs(set[nr].nurs[i]);
    }
    /* reposition the internal drawing nodes */
    if(anz->e>0)
    {
      adjustDrawNodes(0);
      //makeSurfaces();
      updateDispLists();
    }
    if(set[set_bsur].anz_s)
      printf("WARNING: %d surfaces could not be filled, see set:%s\n", set[i].anz_s,specialset->bsur);
    delSet(specialset->bsur);
  }
  else if (compareStrings(type, "RNAM")>0)
  {
    sscanf(string, "%*s %s%s", name, xbuf );
    setNr=getSetNr(xbuf);
    if (setNr>-1)
    {
      printf ("ERROR: set:%s exist already\n", xbuf);
      goto checkForError;;
    }

    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf ("ERROR: set:%s does not exist\n", name);
    }
    else
    {
      printf(" rename %s", set[setNr].name );
      rnam(setNr, xbuf ); 
      printf(" to %s \n ", set[setNr].name ); 
    }
  }
  else if (compareStrings(type, "ROT")>0)
  {
    pre_rot( &string[na+1] );
    // redraw re-sets an eventual sequence
    i=valuestackFlag;
    valuestackFlag=0;
    redraw();
    valuestackFlag=i;
  }
  else if ( compareStrings(type, "SAVE")>0 )
  {
    strcpy(setname,datin);
    for (i=strlen(datin); i>=0; i--)
    {
      if(datin[i]=='.')break;
    }
    setname[i]='\0';
#ifdef WIN32
    sprintf (buffer, "move /y \"%s\" \"%s.fbb\"", datin, setname );
#else
    sprintf (buffer, "mv %s %s.fbb", datin, setname );
#endif
    if(compare(&datin[i+1],"fbd",3)==3) system (buffer);
    pre_write(" all fbd ");
    if( compareStrings( setname, "all")<1 )
    {
#ifdef WIN32
      sprintf (buffer, "move /y all.fbd \"%s.fbd\"", setname );
#else
      sprintf (buffer, "mv all.fbd %s.fbd", setname );
#endif
      system (buffer);
    }     
  }
  else if ( compareStrings(type, "SCAL")>0 )
  {
    entity[0]=0; x=0.;
    length=sscanf(&string[na+1],"%s %lf", entity, &x);
    /* vector length */
    if(entity[0]=='v')
    {
      if(x!=0.)  v_scale*=x;
      else v_scale=1.;
      updateDispLists();
    }
    else if(entity[0]=='d') 
    {
      if(x!=0.) anim_faktor*=x;
      else  anim_faktor=1.;
      if(addDispFlag==1)
      {
         addDispToCoordinates(node);
         addDispToCoordinates(node);
      }
      redraw();
    }
    else if(entity[0]=='s') 
    {
      scale->smin*=x; scale->smax*=x;
      redraw();
    }
    else printf("type:%s not known, factor:%lf\n", entity, x);
  }
  else if (compareStrings(type, "SEND")>0)
  {
    pre_write( &string[na+1] );
  }
  else if (compareStrings(type, "SEQA")>0)
  {
    /* if the name is in use and no valid data are spec. then the seq will be overwritten */
    /* if the name is in use and valid data are spec. then the command is ignored */

    /* for the moment the commands AFTE and BEFO are wrong interpreted */

    /* determine the commands */
    na= sscanf( string, "%*s %s%s%s%s ", name, dat[0],dat[1],dat[2]);
    /* search for valid commands */
    if((dat[0][0]=='A')||(dat[0][0]=='a'))
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='%'; }
      operateAlias( name, "se" );
      if( na < 4) goto checkForError;
      strcpy(comm,"AFTE"); 
      if (addFlag) { for(j=strlen(dat[1]); j>=0; j--) dat[1][j+1]=dat[1][j]; dat[1][0]='%'; }
      operateAlias( dat[1], "p" );
      strcpy(posn,dat[1]); 
      strcpy(entity,dat[2]);
      strcpy(format
      , "%*s%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s");
    }
    else if((dat[0][0]=='B')||(dat[0][0]=='b'))
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='%'; }
      operateAlias( name, "se" );
      if( na < 4) goto checkForError;
      strcpy(comm,"BEFO"); 
      if (addFlag) { for(j=strlen(dat[1]); j>=0; j--) dat[1][j+1]=dat[1][j]; dat[1][0]='%'; }
      operateAlias( dat[1], "p" );
      strcpy(posn,dat[1]); 
      strcpy(entity,dat[2]);
      strcpy(format
      , "%*s%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
    }
    else if ((dat[0][0]=='E')||(dat[0][0]=='e'))
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='%'; }
      operateAlias( name, "se" );
      if( na < 3) goto checkForError;
      strcpy(comm,"END"); 
      strcpy(entity,dat[1]); 
      strcpy(format
      , "%*s%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
    }
    else
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='!'; }
      operateAlias( name, "se" );
      if( na < 3) goto checkForError;
      strcpy(comm,"NEW"); 
      strcpy(entity,dat[0]); 
      strcpy(format
      , "%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
    }

    j=anz_seq=0;
  newline:;
    na =   sscanf( string,format
              , dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9]
              , dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);
    anz_seq+=na;
    if( (seq = (char **)realloc((char **)seq, (anz_seq)*sizeof(char *))) == NULL )
    { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
    for (i=anz_seq-na; i<anz_seq; i++)
    {
      if( (seq[i]= (char *)malloc( MAX_LINE_LENGTH*sizeof(char))) == NULL )
      { printf(" ERROR: malloc failure in interpreter()\n\n"); return(-1); }
    }

    /* look if the command-line continues */
    for (i=0; i<na; i++)
    {
      /* printf("dat:%d |%s|\n",i, dat[i]); */
      if(dat[i][0]=='=')
      {
        length = frecord( handle1, string);
        strcpy(format
        , "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
        goto newline;
      }
      if (addFlag) { for(l=strlen(dat[i]); l>=0; l--) dat[i][l+1]=dat[i][l]; dat[i][0]='%'; }
      operateAlias( dat[i], "p" );
      strcpy(seq[j], dat[i]);
      j++;
    }

    if((entity[0]=='N')||entity[0]=='n') type[0]='n';
    else if((entity[0]=='P')||entity[0]=='p') type[0]='p';
    else 
    {
      errMsg(" ERROR in SEQA: entity not known\n" );
      goto checkForError;
    }
    /* mark the set as a sequence-set */
    type[1]='s';

    if(comm[0]=='A')
    {
      /* the set exists */
      setNr=getSetNr( name );
      if (setNr<0)
      {
        printf (" ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }

      if(type[0]=='p') ibuf=set[setNr].anz_p+anz_seq;
      if(type[0]=='n') ibuf=set[setNr].anz_n+anz_seq;
      if((buf=(char **)malloc(ibuf*sizeof(char *)))==NULL)
      { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      for (i=0; i<ibuf; i++)
      {
        if( (buf[i]= (char *)malloc( MAX_LINE_LENGTH*sizeof(char))) == NULL )
        { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      }

      j=0;
      if( type[0]=='p')
      {
       for (i=0; i<set[setNr].anz_p; i++)
       {
        strcpy( buf[j], point[set[setNr].pnt[i]].name); j++;
        if( compareStrings( point[set[setNr].pnt[i]].name, posn)>0)
        {
          for (k=0; k<anz_seq; k++) { strcpy( buf[j], seq[k]); j++; }
        }
       }
       set[setNr].anz_p = 0;
      }     
      if( type[0]=='n')
      {
       for (i=0; i<set[setNr].anz_n; i++)
       {
        sprintf( buf[j],"%d", set[setNr].node[i]);
        if( compareStrings( buf[j], posn)>0)
        {
          for (k=0; k<anz_seq; k++) { j++; strcpy( buf[j], seq[k]); }
        }
        j++;
       }
       set[setNr].anz_n = 0;
      }     
      for (i=0; i<j; i++)
      {
        pre_seta( name, type, buf[i]);
      }

      for (i=0; i<ibuf; i++) free(buf[i]); free(buf);
    }
    else if(comm[0]=='B')
    {
      /* the set exists */
      setNr=getSetNr( name );
      if (setNr<0)
      {
        printf (" ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }

      if(type[0]=='p') ibuf=set[setNr].anz_p+anz_seq;
      if(type[0]=='n') ibuf=set[setNr].anz_n+anz_seq;
      if((buf=(char **)malloc(ibuf*sizeof(char *)))==NULL)
      { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      for (i=0; i<ibuf; i++)
      {
        if( (buf[i]= (char *)malloc( MAX_LINE_LENGTH*sizeof(char))) == NULL )
        { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      }

      j=0;
      if( type[0]=='p')
      {
       for (i=0; i<set[setNr].anz_p; i++)
       {
        if( compareStrings( point[set[setNr].pnt[i]].name, posn)>0)
        {
          for (k=0; k<anz_seq; k++) { strcpy( buf[j], seq[k]); j++; }
        }
        strcpy( buf[j], point[set[setNr].pnt[i]].name); j++;
       }
       set[setNr].anz_p = 0;
      }     
      if( type[0]=='n')
      {
       for (i=0; i<set[setNr].anz_n; i++)
       {
        sprintf( xbuf,"%d", set[setNr].node[i]);
        if( compareStrings( xbuf, posn)>0)
        {
          for (k=0; k<anz_seq; k++) { strcpy( buf[j], seq[k]); j++; }
        }
        sprintf( buf[j],"%d", set[setNr].node[i]); j++;
       }
       set[setNr].anz_n = 0;
      }     
      for (i=0; i<j; i++)
      {
        pre_seta( name, type, buf[i]);
      }

      for (i=0; i<ibuf; i++) free(buf[i]); free(buf);
    }
    else if(comm[0]=='E')
    {
      for (i=0; i<anz_seq; i++)
      {
        pre_seta( name, type, seq[i]);
      }
    }
    else if(comm[0]=='N')
    {
      delSet(name);
      for (i=0; i<anz_seq; i++)
      {
        pre_seta( name, type, seq[i]);
      }
    }

    for (i=0; i<anz_seq; i++) free(seq[i]);
  }
  else if (compareStrings(type, "SEQC")>0)
  {
    na= sscanf( string, "%*s %s", name);
    setNr=getSetNr(name);
    if(setNr<0)
    {
      printf (" set:%s does not exist\n", name);
      goto checkForError;
    }
    if( (picnr = (int *)realloc( (int *)picnr, (set[setNr].anz_c) * sizeof(int))) == NULL )
    {  printf("\n\n ERROR: realloc failed, hcpy\n\n"); }
    for(i=0; i<set[setNr].anz_c; i++) picnr[i]=set[setNr].lcmb[i];
    pics=set[setNr].anz_c;
    l=1;
    if(set[setNr].flag=='c') seto(set[setNr].name);
    else l=0;
    for(i=0; i<pics; i++) convertLCMB( picnr[i]);
    if(l) setc(set[setNr].name);
  }
  else if (compareStrings(type, "SEQL")>0)
  {
    k=0;
    na= sscanf( string, "%*s %s %d", name, &k);
    setNr=getSetNr(name);
    if(setNr<0)
    {
      printf (" set:%s does not exist\n", name);
      goto checkForError;
    }
    if(k<0)
    {
      printf (" the nr of spline-points must be positive\n");
      goto checkForError;
    }
    if(k==0)
    {
      for(i=0; i<set[setNr].anz_l; i++) { line[set[setNr].line[i]].typ=' '; line[set[setNr].line[i]].trk=-1; repLine(set[setNr].line[i]); }
      goto checkForError;
    }
    l=1;
    if(set[setNr].flag=='c') seto(set[setNr].name);
    else l=0;
    for(i=0; i<set[setNr].anz_l; i++) convertLine( set[setNr].line[i],k);
    if(l) setc(set[setNr].name);
  }
  else if (compareStrings(type, "SETA")>0)
  {
    na =   sscanf( string, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               name, type, dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9],
               dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);

    /* create sets for disjunct meshes */ 
    if((strlen(name)==1)&&(name[0]=='!'))
    {
      if(type[0]=='e') separateMeshes(dat[0], specialset->cf);
      else if(type[0]=='l') separateLines(dat[0], specialset->cf);
      else separateMeshes(type, specialset->cf);
      goto checkForError;
    }

    /* check if the setname matches a seqential-set (sq). if yes do not add entities */
    l=getSetNr(name);
    if (l>-1)
    {
      if(set[l].type==1)
      {
        printf("ERROR: setname:%s in use by a sequence. Set can not be created.\n", name);
        goto checkForError;
      }
    }

    /* Single letters (except 'se' 'sh' 'ld') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='v')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='d'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='r')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      /* look if a sequence is defined */
      if((dat[1][0]=='-')&&(dat[1][1]=='\0'))
      {
        if((type[0]!='n')&&(type[0]!='e')) 
        {
          printf("ERROR: sequence only possible for nodes and elements. Set %s can not be created.\n", name);
          goto checkForError;
        }
        for(i=atoi(dat[0]); i<=atoi(dat[2]); i+=atoi(dat[3]) )
        {
          sprintf(buffer,"%d",i);
          pre_seta( name, type, buffer);
        }
      }
      else
      {
        for (i=0; i<na-2; i++)
        {
          if (addFlag) { for(l=strlen(dat[i]); l>=0; l--) dat[i][l+1]=dat[i][l]; dat[i][0]='%'; }
          operateAlias( dat[i], type );

          /* try to add entity of type to the set, if it fails the entity might be a set itself */
          if( pre_seta( name, type, dat[i]) == -1)
	  {
            /* look if a set of this name exists */
            nr=getSetNr(dat[i]);
            if(nr<0)
            {
              /* set was not found. check if wildcards (*) were used */
              length= strsplt( dat[i], '*', &substring);
              if ((length>0)&&(strstr(dat[i], "*") !=NULL))
              {
                for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
                {
                  foundSubString=0;
                  for(l=0; l<length; l++)
                  {
                    if(strstr(set[setNr].name, substring[l]) !=NULL)
                    {
                      foundSubString++;
                      /* check if the first or the last char is no '*' then the substring[] must be at start or end */
          	      if(l==0) { if(dat[i][0]!='*')  { if(dat[i][0]!=set[setNr].name[0])  foundSubString--; }  }
               	      if(l==length-1) { if(dat[i][strlen(dat[i])-1]!='*') { if(dat[i][strlen(dat[i])-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
            	    }
            	  }
                  if(foundSubString==length)
                  {
                    nr=setNr;
	            printf(" set:%s matches %s\n", set[nr].name, dat[i]);
                    if( pre_seta( name, type, set[nr].name ) == -1)
	            {
                        k=pre_seta( name, "i", 0);
                        if ( type[0] == 'n' )        for(j=0; j<set[nr].anz_n; j++) seta(k,type,set[nr].node[j]);
                        else if ( type[0] == 'e' )   for(j=0; j<set[nr].anz_e; j++) seta(k,type,set[nr].elem[j]);
                        else if ( type[0] == 'f' )   for(j=0; j<set[nr].anz_f; j++) seta(k,type,set[nr].face[j]);
                        else if ( type[0] == 'v' )   for(j=0; j<set[nr].anz_v; j++) seta(k,type,set[nr].valu[j]);
                        else if ( type[0] == 'p' )   for(j=0; j<set[nr].anz_p; j++) seta(k,type,set[nr].pnt[j]);
                        else if ( type[0] == 'l' )
			{
                          if((type[1]=='d')&&(type[2]!='\0'))
			  {
                            for(j=0; j<set[nr].anz_l; j++) if(line[set[nr].line[j]].div>=atoi(&type[2])) seta(k,type,set[nr].line[j]);
			  }
			  else if(type[1]!='\0') { for(j=0; j<set[nr].anz_l; j++) seta(k,type,set[nr].line[j]); }
			}
                        else if ( type[0] == 'c' )   for(j=0; j<set[nr].anz_c; j++) seta(k,type,set[nr].lcmb[j]);
                        else if (( type[0] == 's' )&&(type[1]=='\0'))   for(j=0; j<set[nr].anz_s; j++) seta(k,type,set[nr].surf[j]);
                        else if (( type[0] == 's' )&&(type[1]=='h'))   for(j=0; j<set[nr].anz_sh; j++) seta(k,type,set[nr].shp[j]);
                        else if ( type[0] == 'b' )   for(j=0; j<set[nr].anz_b; j++) seta(k,type,set[nr].body[j]);
                        else if ( type[0] == 'L' )   for(j=0; j<set[nr].anz_nurl; j++) seta(k,type,set[nr].nurl[j]);
                        else if ( type[0] == 'S' )   for(j=0; j<set[nr].anz_nurs; j++) seta(k,type,set[nr].nurs[j]);
                    }
                  }
                }
              }
              /* free substring */
              for(j=0; j<length; j++) free(substring[j]);
              free(substring);
            }
            else
            {
		//printf(" entity:%s of type:%c does not exist add contents of set:%d\n", dat[i], type[0], nr);
              k=pre_seta( name, "i", 0);
	      if ( type[0] == 'n' )        for(j=0; j<set[nr].anz_n; j++) seta(k,type,set[nr].node[j]);
	      else if ( type[0] == 'e' )   for(j=0; j<set[nr].anz_e; j++) seta(k,type,set[nr].elem[j]);
	      else if ( type[0] == 'f' )   for(j=0; j<set[nr].anz_f; j++) seta(k,type,set[nr].face[j]);
 	      else if ( type[0] == 'v' )   for(j=0; j<set[nr].anz_v; j++) seta(k,type,set[nr].valu[j]);
 	      else if ( type[0] == 'p' )   for(j=0; j<set[nr].anz_p; j++) seta(k,type,set[nr].pnt[j]);
              else if ( type[0] == 'l' )
	      {
                if((type[1]=='d')&&(type[2]!='\0'))
	        {
                  for(j=0; j<set[nr].anz_l; j++) if(line[set[nr].line[j]].div>=atoi(&type[2])) seta(k,type,set[nr].line[j]);
	        }
	        else if(type[1]=='\0') { for(j=0; j<set[nr].anz_l; j++) seta(k,type,set[nr].line[j]); }
		else printf(" ERROR: Parameter:%s unknown\n",type);
	      }
 	      else if ( type[0] == 'c' )   for(j=0; j<set[nr].anz_c; j++) seta(k,type,set[nr].lcmb[j]);
 	      else if (( type[0] == 's' )&&(type[1]=='\0'))   for(j=0; j<set[nr].anz_s; j++) seta(k,type,set[nr].surf[j]);
 	      else if (( type[0] == 's' )&&(type[1]=='h'))   for(j=0; j<set[nr].anz_sh; j++) seta(k,type,set[nr].shp[j]);
 	      else if ( type[0] == 'b' )   for(j=0; j<set[nr].anz_b; j++) seta(k,type,set[nr].body[j]);
 	      else if ( type[0] == 'L' )   for(j=0; j<set[nr].anz_nurl; j++) seta(k,type,set[nr].nurl[j]);
 	      else if ( type[0] == 'S' )   for(j=0; j<set[nr].anz_nurs; j++) seta(k,type,set[nr].nurs[j]);
	    }
          }
        }
      }

      if((type[0]=='s')&&(type[1]=='e'))
      {
        completeSet( name, "do");
      }
    }
    else
    {
      /* items of unknown type, determine each type */
      if (addFlag) { for(l=strlen(type); l>=0; l--) type[l+1]=type[l]; type[0]='%'; }
      ibuf=0;
      operateAlias( type, "v" ); if (getValuNr(type) >-1)  {  pre_seta( name, "v", type); goto next_seta;  } 
      operateAlias( type, "b" ); if (getBodyNr(type) >-1)  {  pre_seta( name, "b", type); goto next_seta;  } 
      operateAlias( type, "s" ); if (getSurfNr(type) >-1)  {  pre_seta( name, "s", type); goto  next_seta;  } 
      operateAlias( type, "c" ); if (getLcmbNr(type) >-1)  {  pre_seta( name, "c", type); goto  next_seta;  } 
      operateAlias( type, "l" ); if (getLineNr(type) >-1)  {  pre_seta( name, "l", type); goto  next_seta;  } 
      operateAlias( type, "p" ); if (getPntNr(type) >-1)   {  pre_seta( name, "p", type); goto  next_seta;  } 
      operateAlias( type, "se" ); if (getSetNr(type) >-1)   {  pre_seta( name, "se", type); ibuf=1; goto  next_seta;  } 
      operateAlias( type, "sh" ); if (getShapeNr(type) >-1) {  pre_seta( name, "sh", type); ibuf=1; goto  next_seta;  } 
      next_seta:;

      for (i=0; i<na-2; i++)
      {
        if (addFlag) { for(l=strlen(dat[i]); l>=0; l--) dat[i][l+1]=dat[i][l]; dat[i][0]='%'; }
        operateAlias( dat[i], "v" ); if (getValuNr(dat[i]) >-1)  { pre_seta( name, "v",dat[i] ); goto next_dat;  } 
        operateAlias( dat[i], "b" ); if (getBodyNr(dat[i]) >-1)  { pre_seta( name, "b",dat[i] ); goto next_dat;  } 
        operateAlias( dat[i], "s" ); if (getSurfNr(dat[i]) >-1)  { pre_seta( name, "s",dat[i] ); goto  next_dat;  } 
        operateAlias( dat[i], "c" ); if (getLcmbNr(dat[i]) >-1)  { pre_seta( name, "c",dat[i] ); goto  next_dat;  } 
        operateAlias( dat[i], "l" ); if (getLineNr(dat[i]) >-1)  { pre_seta( name, "l",dat[i] ); goto  next_dat;  }
        operateAlias( dat[i], "p" ); if (getPntNr(dat[i]) >-1)   { pre_seta( name, "p",dat[i] ); goto  next_dat;  } 
        operateAlias( dat[i], "se" ); if (getSetNr(dat[i]) >-1)   {  pre_seta( name, "se",dat[i]); ibuf=1; goto  next_dat;  } 
        operateAlias( dat[i], "sh" ); if (getShapeNr(dat[i]) >-1) {  pre_seta( name, "sh",dat[i]); ibuf=1; goto  next_dat;  } 
      next_dat:;
      }
      if(ibuf)
      {
        completeSet( name, "do");
      }
    }
  }
  else if (compareStrings(type, "SETC")>0)
  {
    for(i=0; i<MAX_LINE_LENGTH; i++) name[i]=0;
    length=sscanf( string, "%*s%s", name);
    setc(name);
  }
  else if (compareStrings(type, "SETE")>0)
  {
    for(i=0; i<MAX_LINE_LENGTH; i++) name[i]=0;
    strcpy(comm,"strict");
    length=sscanf( string, "%*s%s%s%s", name, type, comm);

    /* Single letters (except 'se' 'sh') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='r')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      setNr=getSetNr(name);
      if (setNr==-1)
      {
        printf("ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }
      sete(setNr,type,comm);
    }
    else
    {
      printf(" ERROR: Type of data to be enquired has to be stated\n");
      goto checkForError;
    }
  }
  else if (compareStrings(type, "SETI")>0)
  {
    na =   sscanf( string, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               name, type, dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9],
               dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);

    /* check if the setname matches a seqential-set (sq). if yes do not add entities */
    l=getSetNr(name);
    if (l>-1)
    {
      if(set[l].type==1)
      {
        printf("ERROR: setname:%s in use by a sequence. Set can not be created.\n", name);
        goto checkForError;
      }
    }
    for (i=0; i<na-2; i++)
    {
      operateAlias( dat[i], type );
      l=getSetNr(dat[i]);
      if (l==-1)
      {
        printf("ERROR: set:%s does not exist\n", dat[i]);
        goto checkForError;
      }
    }

    /* Single letters (except 'se' 'sh') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='r')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      /* add the spec type to set 'name' if its included in all spec. sets (dat) */
      setNr=pre_seta( name, "i", 0);
      seti(setNr,type,na-2, dat);
    }
    else
    {
      printf(" ERROR: Type of data to be intersected has to be stated\n");
      goto checkForError;
    }
  }
  else if (compareStrings(type, "SETL")>0)
  {
    type[0]=name[0]='\0';
    length=sscanf( string, "%*s%s%s", name, type);
    setNr=getSetNr( name );
    if (setNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", name );
      goto checkForError;
    }
    if(type[0]=='u') set[setNr].lock=0;
    else set[setNr].lock=1;
  }
  else if (compareStrings(type, "SETO")>0)
  {
    name[0]='\0';
    length=sscanf( string, "%*s%s", name);
    seto(name);
  }
  else if (compareStrings(type, "SETR")>0)
  {
    na =   sscanf( string, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               name, type, dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9],
               dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);

    operateAlias( name, "se" );
    setNr=getSetNr( name );
    if (setNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", name );
      goto checkForError;
    }
    /* Single letters (except 'se' 'sh') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='v')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='l'))||
        ((type[0]=='l')&&(type[1]=='a'))||
        ((type[0]=='l')&&(type[1]=='s'))||
        ((type[0]=='l')&&(type[1]=='n'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      for (i=0; i<na-2; i++)
      {
        operateAlias( dat[i], type );
        nr=getSetNr(dat[i]);
        if(nr>-1)
        {
          if ((type[0]=='s')&&(type[1]=='e'))
          {
            /* remove the contents of nr from setNr */
            for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
            for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
            for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
            for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
            for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );
            for (j=0; j<set[nr].anz_l; j++) setr( setNr, "l", set[nr].line[j] );
            for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
            for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] );
            for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );
            for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
            for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
            for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
            for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
          }
          else if (type[0]=='v') for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
          else if (type[0]=='n') for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
          else if (type[0]=='e') for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
          else if (type[0]=='f') for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
          else if (type[0]=='p') for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );	
          else if (type[0]=='l')
	  {
            for (j=0; j<set[nr].anz_l; j++)
	    {
              if((type[1]=='\0')||(type[1]==line[set[nr].line[j]].typ)||((type[1]=='l')&&(line[set[nr].line[j]].typ==32))) setr( setNr, "l", set[nr].line[j] );
	    }
	  }
          else if (type[0]=='c') for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
          else if ((type[0]=='s')&&(type[1]=='\0')) for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] ); 
          else if (type[0]=='b') for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );   
          else if (type[0]=='L') for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
          else if (type[0]=='S') for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
          else if ((type[0]=='s')&&(type[1]=='h')) for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
          else if (type[0]=='r') for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
        }
        else
	{
          if(strstr(dat[i], "*") !=NULL)
          {
            /* set was not found. check if wildcards (*) were used */
            length= strsplt( dat[i], '*', &substring);
            if (length>0)
            {
              for(nr=0; nr<anz->sets; nr++) if(set[nr].name!=(char *)NULL)
              {
                foundSubString=0;
                for(l=0; l<length; l++)
                {
                  if(strstr(set[nr].name, substring[l]) !=NULL)
                  {
                    foundSubString++;
                    /* check if the first or the last char is no '*' then the substring[] must be at start or end */
        	      if(l==0) { if(dat[i][0]!='*')  { if(dat[i][0]!=set[nr].name[0])  foundSubString--; }  }
             	      if(l==length-1) { if(dat[i][strlen(dat[i])-1]!='*') { if(dat[i][strlen(dat[i])-1]!=set[nr].name[strlen(set[nr].name)-1])  foundSubString--; } }
                  }
          	}
                if(foundSubString==length)
                {
	          printf(" set:%s matches %s\n", set[nr].name, dat[i]);

                  operateAlias( set[nr].name, type );
                  nr=getSetNr(set[nr].name);
                  if(nr>-1)
                  {
                    if ((type[0]=='s')&&(type[1]=='e'))
                    {
                      /* remove the contents of nr from setNr */
                      for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
                      for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
                      for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
                      for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
                      for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );
                      for (j=0; j<set[nr].anz_l; j++) setr( setNr, "l", set[nr].line[j] );
                      for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
                      for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] );
                      for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );
                      for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
                      for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
                      for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
                      for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
                    }
                    else if (type[0]=='v') for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
                    else if (type[0]=='n') for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
                    else if (type[0]=='e') for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
                    else if (type[0]=='f') for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
                    else if (type[0]=='p') for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );	
                    else if (type[0]=='l') for (j=0; j<set[nr].anz_l; j++) setr( setNr, "l", set[nr].line[j] );
                    else if (type[0]=='c') for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
                    else if ((type[0]=='s')&&(type[1]=='\0')) for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] ); 
                    else if (type[0]=='b') for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );   
                    else if (type[0]=='L') for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
                    else if (type[0]=='S') for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
                    else if ((type[0]=='s')&&(type[1]=='h')) for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
                    else if (type[0]=='r') for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
                  }
                }
              }
              /* free substring */
              for(i=0; i<length; i++) free(substring[i]);
              free(substring);
            }
          }
          else
	  {
            if (type[0]=='n') nr=atoi(dat[i]);
            else if (type[0]=='e') nr=atoi(dat[i]);
            else if (type[0]=='f') nr=atoi(dat[i]);
            else if (type[0]=='v') nr=getValuNr(dat[i]);
            else if (type[0]=='p') nr=getPntNr(dat[i]);
            else if (type[0]=='l') nr=getLineNr(dat[i]);
            else if (type[0]=='c') nr=getLcmbNr(dat[i]);
            else if ((type[0]=='s')&&(type[1]=='\0')) nr=getSurfNr(dat[i]);
            else if (type[0]=='b') nr=getBodyNr(dat[i]);
            else if (type[0]=='L') nr=getNurlNr(dat[i]);
            else if (type[0]=='S') nr=getNursNr(dat[i]);
            else if ((type[0]=='s')&&(type[1]=='h')) nr=getShapeNr(dat[i]);
            else if (type[0]=='r') nr=getSetNr(dat[i]);
            setr( setNr, type, nr);
	  }
        }
      }
    }
    else
    {
      /* items of unknown type, determine each type */
      if (getValuNr(type) >-1)
      { setr( setNr, "v", getValuNr(type)); } 
      if (getBodyNr(type) >-1)
      { operateAlias( type, "b" ); setr( setNr, "b", getBodyNr(type)); } 
      if (getSurfNr(type) >-1)
      { operateAlias( type, "s" ); setr( setNr, "s", getSurfNr(type)); } 
      if (getLcmbNr(type) >-1)
      { operateAlias( type, "c" ); setr( setNr, "c", getLcmbNr(type)); } 
      if (getLineNr(type) >-1)
      { operateAlias( type, "l" ); setr( setNr, "l", getLineNr(type)); } 
      if (getPntNr(type) >-1)
      { operateAlias( type, "p" ); setr( setNr, "p", getPntNr(type)); } 
      if (getShapeNr(type) >-1)
      { operateAlias( type, "sh" ); setr( setNr, "sh", getShapeNr(type)); } 
      for (i=0; i<na-2; i++)
      {
        if (getValuNr(dat[i]) >-1)
        { setr( setNr, "v", getValuNr(dat[i]) ); } 
        if (getBodyNr(dat[i]) >-1)
        { operateAlias( dat[i], "b" ); setr( setNr, "b", getBodyNr(dat[i]) ); } 
        if (getSurfNr(dat[i]) >-1)
        { operateAlias( dat[i], "s" ); setr( setNr, "s", getSurfNr(dat[i]) ); } 
        if (getLcmbNr(dat[i]) >-1)
        { operateAlias( dat[i], "c" ); setr( setNr, "c", getLcmbNr(dat[i]) ); } 
        if (getLineNr(dat[i]) >-1)
        { operateAlias( dat[i], "l" ); setr( setNr, "l", getLineNr(dat[i]) ); }
        if (getPntNr(dat[i]) >-1)
        { operateAlias( dat[i], "p" ); setr( setNr, "p", getPntNr(dat[i]) ); } 
        if (getShapeNr(dat[i]) >-1)
        { operateAlias( dat[i], "sh" ); setr( setNr, "sh", getShapeNr(dat[i]) ); } 
      }
    }
  }
  else if (compareStrings(type, "SHPE")>0)
  {
    pre_shape( &string[na+1], addFlag);
  }
  else if (compareStrings(type, "SPLIT")>0)
  {
    pre_split( &string[na+1] );
    realloc_colNr();
    redraw();
  }
  else if (compareStrings(type, "STACK")>0)
  {
    if(compare( &string[na+1],"on",2)==2 ) valuestackFlag=1;
    if(compare( &string[na+1],"off",2)==2 ) valuestackFlag=0;
    if(compare( &string[na+1],"free",2)==2 )
    {
      for(i=0; i<valuestack_ptr; i++)  free(valuestack[i]);
      valuestack_ptr=0;
      free(valuestack);
      valuestack=NULL;
    }
  }
  else if (compareStrings(type, "STEPS")>0)
  {
    steps= atoi( &string[na+1]  );
    redraw();
  }
  else if (compareStrings(type, "STOP")>0)
  {
    if(cur_commandFile>-1)
    {
      if(commandFile[cur_commandFile].stopped==0)
      {
        printf("STOP by user request, reading of file:%s interrupted.\n", commandFile[cur_commandFile].name);
        commandFile[cur_commandFile].stopped=1;
	stopped_commandFile=cur_commandFile;
        return(-2);
      }
      else printf("WARNING: commandFile:%s already interrupted\n", commandFile[stopped_commandFile].name);
    }
    else printf("ERROR: No open commandFile\n");
  }
  else if (compareStrings(type, "SUBM")>0)
  {
    pre_subm(&string[na+1]);
  }
  else if (compareStrings(type, "SURF")>0)
  {
    pre_surf(&string[na+1]);
  }
  else if (compareStrings(type, "SWEP")>0)
  {
    pre_swep(&string[na+1]);
    redraw();
  }
  else if (compareStrings(type, "SYS")>0)
  {
    if(allowSysFlag) system(&string[na+1]);
    else
    {
      printf(" WARNING: Found a 'sys' command (a system call):\n%s\n Since a system call can be dangerous You have to chose now between stop (s), continue (c) or enable (e). If you chose 'e' the 'sys' command will be un-locked by creating or extending your personal config file '%s/.cgx' with the un-lock command ALLOW_SYS. You may delete this command from your '.cgx' file to lock the 'sys' command again.\n", string, homepath);
      printf(" Waiting for user input. Please type into the terminal either s,c or e (the terminal has to be the active window!):\n");
      i=1;
      do
      {
        scanf("%c%c",&cbuf,(char *)&ibuf);
        if(cbuf=='s') exit(0);
        else if(cbuf=='c') { i=0; system(&string[na+1]); }
        else if(cbuf=='e') { i=0; allowSysFlag=1; addCommandToInitFile("ALLOW_SYS"); }
        else { printf(" ERROR:%c not valid\n",cbuf); }
      }while(i);
    }
  }
  else if (compareStrings(type, "TEST")>0)
  {
    length=sscanf( string, "%*s %s %s", type, name);

    if ((type[0]=='i')&&(type[1]=='\0'))
    {
      length=sscanf( string, "%*s%*s%*s%s", dat[0]);
      operateAlias( name, "se" );
      operateAlias( dat[0], "se" );
  
      setNr=getSetNr(name);
      if(setNr==-1)
      {
        printf("ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }
      nr=getSetNr(dat[0]);
      if(nr==-1)
      {
        printf("ERROR: set:%s does not exist\n", dat[0]);
        goto checkForError;
      }
      else completeSet(set[nr].name, "do") ;
      
      na = embodies(setNr,nr, &cof[0]);
      if(na>=1)
      {
	strcpy(parameter[0],"TRUE");
        printf(" %s\n",parameter[0]);
        if(printFlag) printf(" node:%d coef:%e %e %e %e\n",na, cof[0], cof[1], cof[2], cof[3]);
        /* only senseful if embodies() would return the deepest penetration
        sprintf(parameter[1],"%d",na);
        sprintf(parameter[2],"%e",cof[0]);
        sprintf(parameter[3],"%e",cof[1]);
        sprintf(parameter[4],"%e",cof[2]);
        sprintf(parameter[5],"%e",cof[3]);
        write2stack(6, parameter);
	*/
        write2stack(1, parameter);
      }
      else 
      {
	strcpy(parameter[0],"FALSE");
        printf(" %s\n",parameter[0]);
        write2stack(1, parameter);
      }
      goto checkForError;
    }
    if ((type[0]=='o')&&(type[1]=='\0'))
    {
      operateAlias( name, "se" );
      strcpy(parameter[0],"FALSE");
      i=getSetNr(name);
      if(i>-1)
      {
        if(set[i].anz_n>0)
        {
          if( plotNode(set[i].node[0]) >0) strcpy(parameter[0],"TRUE");
        }
      }
      else
      {
        if( plotNode(atoi(name)) >0) strcpy(parameter[0],"TRUE");
      }
      printf(" %s\n",parameter[0]);
      write2stack(1, parameter);
      goto checkForError;
    }
    operateAlias( name, type );
    strcpy(parameter[0],"FALSE");
    if ((type[0]=='n')&&(type[1]=='\0')) { ibuf=atoi(name);
      if ((ibuf>=anz->nmin)&&(ibuf<=anz->nmax)&&(node[ibuf].indx==0)&&(ibuf==node[node[ibuf].indx].nr)&&(node[ibuf].pflag==0)) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='e')&&(type[1]=='\0')) { ibuf=atoi(name);
      if ((ibuf>=anz->emin)&&(ibuf<=anz->emax)&&(e_enqire[ibuf].type!=0)) strcpy(parameter[0],"TRUE"); }
    //if ((type[0]=='f')&&(type[1]=='\0')) { if( get??Nr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='v')&&(type[1]=='\0')) { if( getValuNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='p')&&(type[1]=='\0')) { if( getPntNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='l')&&(type[1]=='\0')) { if( getLineNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='c')&&(type[1]=='\0')) { if( getLcmbNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    //if ((type[0]=='r')&&(type[1]=='\0')) { if( get??Nr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='s')&&(type[1]=='\0')) { if( getSurfNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='s')&&(type[1]=='e'))  { if( getSetNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='s')&&(type[1]=='h'))  { if( getShapeNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='L')&&(type[1]=='\0')) { if( getNurlNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='S')&&(type[1]=='\0')) { if( getNursNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='b')&&(type[1]=='\0')) { if( getBodyNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    printf(" %s\n",parameter[0]);
    write2stack(1, parameter);
  }
  else if (compareStrings(type, "TRA")>0)
  {
    length=sscanf( string, "%*s %s %lf", posn, &dx);
    if(posn[0]=='u') dty+= dx/scale->w/ds;
    if(posn[0]=='d') dty-= dx/scale->w/ds;
    if(posn[0]=='l') dtx-= dx/scale->w/ds;
    if(posn[0]=='r') dtx+= dx/scale->w/ds;
    if(posn[0]=='f') dtz+= dx/scale->w;
    //printf("dtx:%f dty:%f dtz:%f dx:%f ds:%f\n", dtx, dty, dtz, (dx*2)/scale->w, ds);
    glutPostRedisplay();
    updateDispLists();
  }
  else if (compareStrings(type, "THRS")>0)
  {
    delSet(specialset->thrs);
    sscanf(&string[na+1] , "%s %s %s", xbuf, format, comm);
    // off
    if(xbuf[0]=='o')
    {
      sprintf(buffer,"fill");
      pre_view(buffer);
      sprintf(buffer,"fv all");
      plot(buffer);
      goto checkForError;
    }
    sprintf(buffer,"all %s rec _ _ _ 0. %s %s", specialset->thrs, format, xbuf );
    enquireEntities(buffer);
    completeSet( specialset->thrs, "up");
    // generate node-texts on the max or min nodes in each local element cluster
    if(comm[0]=='t')
    {
      // generate separate sets of all clusters
      strcpy(setname2, "+grp");
      for(i=1; i<=grpSets; i++)
      {
        sprintf(buffer,"%s%d", setname2, i);
        delSet(buffer);
        sprintf(buffer,"%sN%d", setname2, i);
        delSet(buffer);
      }
      grpSets=separateMeshes(specialset->thrs, setname2);
      sprintf(buffer,"point 4");
      pre_view(buffer);
      sprintf(buffer,"elem off");
      pre_view(buffer);
      sprintf(buffer,"ev %s", specialset->thrs);
      plot(buffer);
      // search max/min and generate node text
      for(i=1; i<=grpSets; i++)
      {
        sprintf(buffer,"%s%d %sN%d rec _ _ _ 0. %s", setname2, i, setname2, i, format );
        enquireEntities(buffer);
        sprintf(buffer,"%sN%d", setname2, i);
        setNr=getSetNr(buffer);
        if (setNr<0)
        {
          printf (" txt: set:%s does not exist\n", name);
          goto checkForError;
        }
        for(j=0; j<set[setNr].anz_n; j++)
        {
          createText(set[setNr].node[j], -1, 0 );
        }
        sprintf(buffer,"nt %s k", set[setNr].name);
        plus(buffer);
      }
      printf("\n The command 'qtxt' may be used to modify the node related texts\n");
    }
    else
    {
      sprintf(buffer,"point 4");
      pre_view(buffer);
      sprintf(buffer,"elem off");
      pre_view(buffer);
      sprintf(buffer,"ev %s", specialset->thrs);
      plot(buffer);
    }
  }
  else if (compareStrings(type, "TRFM")>0)
  {
    transformResults(&string[na+1]);
  }
  else if (compareStrings(type, "TEXT")>0)
  {
    printf(" WARNING: The text command was replaced by ulin. Please use ulin in future.\n");
    stos( &string[na], 1, strlen(&string[na]), picture_text);
  }
  else if (compareStrings(type, "TXT")>0)
  {
    placeTxt(&string[na]);
  }
  else if (compareStrings(type, "UCUT")>0)
  {
    uncut(1);
  }
  else if (compareStrings(type, "ULIN")>0)
  {
    stos( &string[na], 1, strlen(&string[na]), picture_text);
    if(inpformat)
    {
      glutSetWindow( w0);
      glutPostRedisplay();
    }
  }
  else if ((compareStrings(type, "VAL")>0)||(compareStrings(type, "VALU")>0))
  {
    pre_value(&string[na+1]);
  }
  else if (compareStrings(type, "VIEW")>0)
  {
    pre_view(&string[na+1]);
  }
  else if (compareStrings(type, "VOLU")>0)
  {
    pre_volu(&string[na+1]);
  }
  else if (compareStrings(type, "WSIZE")>0)
  {
    setWindowSize(string);
  }
  else if (compareStrings(type, "WPOS")>0)
  {
    setWindowPos(string);
  }
  else if (compareStrings(type, "ZAP")>0)
  {
    if( zap(&string[na+1])==-1)
      printf (" ERROR in ZAP: set:%s does not exist\n",&string[na+1] );
  }
  else if (compareStrings(type, "ZOOM")>0)
  {
    length=sscanf( string, "%*s %lf %lf %lf %lf", &wx[0], &wy[0], &wx[1], &wy[1]);
    if (length==4)
    {
      wx[0]*=2.;
      wx[1]*=2.;
      wy[0]*=2.;
      wy[1]*=2.;

      wx[0]+=-1.;
      wx[1]+=-1.;
      wy[0]+=-1.;
      wy[1]+=-1.;
    }
    else
    {
      dx=1./wx[0];
      wx[0]=wy[0]=-dx;
      wx[1]=wy[1]=dx;
    }
    zoom(wx[0], wy[0], wx[1], wy[1]);
  }
  else if (compareStrings(type, "CMAP")>0)
  {
    sscanf(string, "%*s %s", buffer);

    for (i=0; i<cmaps; i++)
    {
      if( compare( cmap_names[i], buffer, 2)==2)
      {
        strcpy(cmap_name, buffer);
        printf("\nSetting \'%s\' colormap.\n\n", cmap_name);
        defineColTextur_load(1.);
        redraw();
        goto checkForError;
      }
    }
    printf("\nColormap \'%s\' not defined.\n\n", buffer);
  }
  else if ((type[0]== '!')||(type[0]== '#')||(type[0]== '$'))
  {
    printf(" %s\n", string );
  }
  else
  {
    printf(" key:%s from string %s not known\n", type, string);
  }

 checkForError:;
  if(returnFlag<0) return(returnFlag);
  if((whileFlag==3)||(whileFlag==5)) goto whileLoop;
  return(returnFlag);
}



int getCommandLine(FILE *handle1, char **string1)
{
  int  i,length;
  char buffer[MAX_LINE_LENGTH];
  char *string;

  string=*string1;

  /* for long commands (>MAX_LINE_LENGTH-1) combine several calls to frecord to build the command which will be stored in 'string' */
  i=0;
 addLine:;
  do
  {
    if( (string=realloc(string, MAX_LINE_LENGTH*++i*sizeof(char)))== NULL ) { printf(" ERROR: realloc failed in getCommandLine()\n"); return(0); }
    length = frecord( handle1, buffer);
    if(i==1) strcpy(string, buffer);
    else strcpy(&string[strlen(string)], buffer);
  }while(length==(MAX_LINE_LENGTH-1));
  //for(i=0; i<strlen(string); i++) printf("char%d %c\n",i,string[i]);
  if((length>0)&&(string[strlen(string)-2]=='='))
  {
    string[strlen(string)-2]=0;
    goto addLine;
  }

  *string1=string;
  return(strlen(string)-1);
}



int readfbd( char *datin, int addFlag )
{
  FILE      *handle1=NULL;
  int       result=0, length, na, gtolFlag=0;
  char type[MAX_LINE_LENGTH];
  static char *string=NULL;

  if(backgroundFlag) inpformat=0;
  else inpformatbuffer=inpformat;
  //printf("3backgroundFlag:%d inpformat:%d inpformatbuffer:%d\n",backgroundFlag,inpformat, inpformatbuffer);
  
  if((datin==0)&&(stopped_commandFile>-1))  // restart reading
  {
    if(commandFile[stopped_commandFile].stopped==1)
    {
      cur_commandFile=stopped_commandFile;
      handle1 = commandFile[cur_commandFile].handle;  // always the last opened 
      commandFile[cur_commandFile].stopped=0;
      addFlag=commandFile[cur_commandFile].addFlag;
      // close and re-open to catch updates in the file
      fclose(handle1);
      handle1 = fopen (commandFile[cur_commandFile].name, "r");
      if (handle1==NULL)
      {
        printf (" ERROR in readfbd: The input file \"%s\" could not be opened.\n\n", commandFile[cur_commandFile].name);
        inpformat=inpformatbuffer;
	//printf("4backgroundFlag:%d inpformat:%d inpformatbuffer:%d\n",backgroundFlag,inpformat, inpformatbuffer);
	return(1);
      }
      if( fsetpos( handle1, (fpos_t *)commandFile[cur_commandFile].filepntr)!=0) { printf("error in fsetpos"); return(-1); }
      commandFile[cur_commandFile].handle=handle1; 
    }
    else printf("ERROR: commandFile:%s was not halted\n", commandFile[stopped_commandFile].name);
  }
  else
  {
    handle1 = fopen (datin, "r");
    if (handle1==NULL)
    {
      printf (" ERROR in readfbd: The input file \"%s\" could not be opened.\n\n", datin);
      inpformat=inpformatbuffer;
      //printf("5backgroundFlag:%d inpformat:%d inpformatbuffer:%d\n",backgroundFlag,inpformat, inpformatbuffer);
      return(1);
    }
    else  printf (" %s opened\n",datin);
  
    /* store the handle */
    if( (commandFile=(CommandFile *)realloc((CommandFile *)commandFile, (commandFiles+1)*sizeof(CommandFile))) == NULL ) printf(" ERROR: malloc failed\n");
    strcpy(commandFile[commandFiles].name,datin);
    commandFile[commandFiles].handle=handle1;
    commandFile[commandFiles].stopped=0;
    commandFile[commandFiles].addFlag=addFlag;
    cur_commandFile=commandFiles;

    if( (commandFile[commandFiles].filepntr=(fpos_t *)malloc(1*sizeof(fpos_t))) == NULL ) printf(" ERROR: malloc failed\n");

    commandFiles++;
    //printf("open cur_commandFile %d files %d\n", cur_commandFile, commandFiles);
  
    printf ("\n reading file\n");
  }
  
  do
  {
    length=getCommandLine(handle1, &string);
    if (length > 0)
    {
      type[0]=0;
      na=0; while(string[na]==' ') na++;
      na+= sword( string, type);
      //printf (" record:%s", &string[0]);
      result=commandoInterpreter( type, string, na, length, handle1, addFlag, &gtolFlag );
      //printf("result%d in %s\n", result, commandFile[cur_commandFile].name);
    }
    if( string[length]== (char)EOF)  break;
  }while (result > -1);

  if((result==-2)||(stopped_commandFile>cur_commandFile))   // a "STOP" command, or a referenced file is stopped
  {
    //printf("%d stopped file: %s\n", cur_commandFile, commandFile[cur_commandFile].name);
    commandFile[cur_commandFile].stopped=1;
    if(fgetpos( commandFile[cur_commandFile].handle, (fpos_t *)commandFile[cur_commandFile].filepntr)!=0) { printf("error in fgetpos");  return(-1); }

    cur_commandFile--;
    updDrawingCube();
  }
  else // it's not a "STOP" command, file wil be closed
  {
    //printf("stopped_commandFile:%d cur_commandFile:%d close %s\n",stopped_commandFile, cur_commandFile, commandFile[cur_commandFile].name);
    fclose(commandFile[cur_commandFile].handle);
    commandFile[cur_commandFile].handle=NULL;
    commandFile[cur_commandFile].stopped=0;
    
    if(cur_commandFile==commandFiles-1) commandFiles--; 

    if((cur_commandFile>0)&&(cur_commandFile==stopped_commandFile))   // restart upper stopped commandFiles
    {
      stopped_commandFile--;
      printf("next stopped_commandFile:%d\n",stopped_commandFile);
      if((commandFile[stopped_commandFile].handle!=NULL)&&(commandFile[stopped_commandFile].stopped))
      {
        printf("CONTINUE to read file:%s\n", commandFile[stopped_commandFile].name);
        readfbd( 0, 0);
        return(0);
      }
    }

    cur_commandFile--;
    if(cur_commandFile==-1)  // last filepointer closed, free struct
    {
      stopped_commandFile--;
      free(commandFile[commandFiles].filepntr);
      free(commandFile);
      commandFile=NULL;
      commandFiles=0;
    }
    result=0;
  }
  
  //if(!gtolFlag) { valuestackFlagbuffer=valuestackFlag; valuestackFlag=0; gtol=calcGTOL(setall); valuestackFlag=valuestackFlagbuffer; }
  // calcGTOL will be executed in pre_read()

  /* delete the entities in specialSet->zap */
  zap(specialset->zap);

  inpformat=inpformatbuffer;
  //printf("6backgroundFlag:%d inpformat:%d inpformatbuffer:%d\n",backgroundFlag,inpformat, inpformatbuffer);
  updDrawingCube();
  redraw();
  printf (" done \n\n");
  return(result);
}


/*------------------------------------------------------------------*/
/* Daten im frd-format einlesen                                     */
/*------------------------------------------------------------------*/
void readfrdfile( char *frdfile, char *setname )
{
  int i, setNr=-1;
  char  datum[MAX_LINE_LENGTH];
  Summen    anzr[1];
  Nodes     *noder=NULL;
  Elements  *elemr=NULL;
  Datasets  *lcaser=NULL;


  if (setname[0]!=0) /* store nodes and elements in set */
  {
    if(printFlag) printf (" read %s in set:%s\n", frdfile, setname );
    readfrd( frdfile, anzr, &noder, &elemr, &lcaser, 1);
  
  
    if(anzr->n>0)
    {
      for (i=0; i<anzr->n; i++)
      {
        if ((noder[i].nr <= anz->nmax)&&(noder[i].nr >= anz->nmin))
        {
          if(setNr<0) { sprintf (datum, "%d", noder[i].nr); setNr=pre_seta( setname, "n", datum ); }
          else seta( setNr, "n", noder[i].nr );
        }
        else printf (" node %d is outside range of known nodes\n", noder[i].nr);
      }
    }
    if(anzr->e>0)
    {
      for (i=0; i<anzr->e; i++)
      {
        if ((elemr[i].nr <= anz->emax)&&(elemr[i].nr >= anz->emin))
        {
          if(setNr<0) { sprintf (datum, "%d", elemr[i].nr); pre_seta( setname, "e", datum ); }
          else seta( setNr, "e", elemr[i].nr );
        }
      else printf (" element %d is outside range of known elements\n", elemr[i].nr);
      }
    }
    free(elemr);
    free(noder);
    free(lcaser);
  }
}



long swap_long(char *ptr)
{
  long buf,buf2;
  char *pbuf, *pbuf2;

  pbuf=(char *)&buf;
  pbuf2=(char *)&buf2;

  *pbuf2=*(ptr);  pbuf2++;
  *pbuf2=*(ptr+1);  pbuf2++;
  *pbuf2=*(ptr+2);  pbuf2++;
  *pbuf2=*(ptr+3); 

  *pbuf=*(ptr+3);  pbuf++;
  *pbuf=*(ptr+2);  pbuf++;
  *pbuf=*(ptr+1);  pbuf++;
  *pbuf=*(ptr);

  // printf("in:%u out:%u\n", buf2, buf);
  return(buf);
}


#define BYTES 100000000
BGpicture *readxwd( char * datin)
{
  int i, j, k, n, fp, offset;
  int bytes_per_line, bits_per_pixel;
  XWDFileHeader *xwdheader;

  static char buf[BYTES];
  static char buf2[BYTES];
  static BGpicture bgpicture;

  char aux;

  fp = open ( datin, O_RDONLY);
  if( read( fp, buf, BYTES)<1)
  {
    printf("ERROR: File:%s could not be opened\n", datin) ;
    return(0);
  }
  xwdheader=(XWDFileHeader *)&buf[0];

  /* swap from big-endian to little-endian */
  printf("header: %d or swaped: %ld\n", xwdheader->header_size, swap_long((char *)&xwdheader->header_size));
  if ((xwdheader->header_size>300)||(xwdheader->header_size<100))
  {
    offset=swap_long((char *)&xwdheader->header_size)+swap_long((char *)&xwdheader->ncolors)*sz_XWDColor;
    bytes_per_line=swap_long((char *)&xwdheader->bytes_per_line);
    bgpicture.width=(GLsizei)swap_long((char *)&xwdheader->window_width);
    bgpicture.height=(GLsizei)swap_long((char *)&xwdheader->window_height);
    bits_per_pixel=(int)swap_long((char *)&xwdheader->bits_per_pixel);

    /*
        printf(" XWD_FILE_VERSION above 	      :%d \n", swap_long(&xwdheader->file_version ));        
        printf(" ZPixmap or XYPixmap 		      :%d \n", swap_long(&xwdheader->pixmap_format ));       
        printf(" Pixmap depth 			      :%d \n", swap_long(&xwdheader->pixmap_depth ));        
        printf(" Pixmap width 	       		       :%d\n", swap_long(&xwdheader->pixmap_width ));        
        printf(" Pixmap height 		      	      :%d \n", swap_long(&xwdheader->pixmap_height ));       
        printf(" Bitmap x offset, normally 0   	      :%d\n", swap_long(&xwdheader->xoffset ));             
        printf(" of image data: MSBFirst, LSBFirs     :%d \n", swap_long(&xwdheader->byte_order ));          
        printf(" bitmap_unit			      :%d \n", swap_long(&xwdheader->bitmap_unit ));
        printf(" bitmaps only: MSBFirst, LSBFirst     :%d\n", swap_long(&xwdheader->bitmap_bit_order ));    
        printf(" bitmap_pad			      :%d \n", swap_long(&xwdheader->bitmap_pad ));
        printf(" Bits per pixel 		      :%d \n", swap_long(&xwdheader->bits_per_pixel ));      
        printf(" bytes_per_line			      :%d \n", swap_long(&xwdheader->bytes_per_line ));
        printf(" Class of colormap 		      :%d \n", swap_long(&xwdheader->visual_class ));        
        printf(" Z red mask			      :%d \n", swap_long(&xwdheader->red_mask ));            
        printf(" Z green mask			      :%d \n", swap_long(&xwdheader->green_mask ));          
        printf(" Z blue mask 			      :%d \n", swap_long(&xwdheader->blue_mask ));           
        printf(" Log2 of distinct color values	      :%d \n", swap_long(&xwdheader->bits_per_rgb ));        
        printf(" Number of entries in colormap; not used? :%d  \n", swap_long(&xwdheader->colormap_entries ));    
        printf(" Number of XWDColor structures	       :%d\n", swap_long(&xwdheader->ncolors ));             
        printf(" Window width			      :%d \n", swap_long(&xwdheader->window_width ));        
        printf(" Window height			      :%d \n", swap_long(&xwdheader->window_height ));       
        printf(" Window upper left X coordinate	      :%d \n", swap_long(&xwdheader->window_x ));            
        printf(" Window upper left Y coordinate	      :%d \n", swap_long(&xwdheader->window_y ));            
        printf(" Window border width                  :%d \n", swap_long(&xwdheader->window_bdrwidth ));     
    */
  }
  else
  {
    offset=xwdheader->header_size + xwdheader->ncolors*sz_XWDColor;
    bytes_per_line=xwdheader->bytes_per_line;
    bgpicture.width=(GLsizei)xwdheader->window_width;
    bgpicture.height=(GLsizei)xwdheader->window_height;
    bits_per_pixel=(int)xwdheader->bits_per_pixel;

    printf("ncolors:%d\n", xwdheader->ncolors);
  }

  bgpicture.name=&buf[100];
  printf("windowName:%s\n", bgpicture.name);
  if(bits_per_pixel==32) { bgpicture.format=GL_RGBA;  bgpicture.type=GL_UNSIGNED_BYTE; }
  if(bits_per_pixel==24) { bgpicture.format=GL_RGB;   bgpicture.type=GL_UNSIGNED_BYTE; }
  if(bits_per_pixel==8)  { bgpicture.format=GL_RGBA;  bgpicture.type=GL_UNSIGNED_BYTE; }
  if(bits_per_pixel==1)  { bgpicture.format=GL_COLOR_INDEX;  bgpicture.type=GL_BITMAP; }


  /* inverse the lines (last is first) */
  n=0;
  for(j=0; j<bgpicture.height; j++)
  {
    for(i=0; i<bytes_per_line; i++)
    {
      buf2[n]=buf[offset-1+ (bytes_per_line*(bgpicture.height-j-1)) +i +1];
      if(bits_per_pixel==8)
      {
        for (k=1; k<4; k++) buf2[n+k]=buf2[n];
        n+=3;
      }
      n++;
    }
  }
  /* convert the image data from RGBA to BGRA if 32bit picture */
  if (bits_per_pixel == 32) for (i=0; i < bgpicture.width * bgpicture.height * bits_per_pixel/8 ; i+= bits_per_pixel/8)
  {
      aux = buf2[i];
      buf2[i] = buf2[i+2];
      buf2[i+2] = aux;
  }

  bgpicture.pixels=&buf2[0];
  printf("pixel-offset:%d\n", offset);
  printf("bits_per_pixel:%d\n", bits_per_pixel);
  printf("bytes_per_line:%d\n", bytes_per_line);
  printf("window_width:%d\n", bgpicture.width);
  printf("window_height:%d\n", bgpicture.height);
  /* printf("window-pixels:%x\n", bgpicture.pixels); */

  return(&bgpicture);
}

BGpicture *_readxwd(void)
{
   int i, j, c;
   static GLubyte checkImage[64][64][3];
   static BGpicture bgpicture;
   static char name[100];

   for (i = 0; i < 64; i++) {
      for (j = 0; j < 64; j++) {
         c = ((((i&0x8)==0)^((j&0x8)))==0)*255;
         checkImage[i][j][0] = (GLubyte) c/(i+1);
         checkImage[i][j][1] = (GLubyte) c/(j+1);
         checkImage[i][j][2] = (GLubyte) c;
      }
   }
  strcpy(name,"CheckImage");

  bgpicture.name=&name[0];
  bgpicture.width=64;
  bgpicture.height=64;
  bgpicture.format=GL_RGB;
  bgpicture.type=GL_UNSIGNED_BYTE;
  bgpicture.pixels=&checkImage;

  return(&bgpicture);
}



void readtostack(char *datin)
{
  int i, length, lines;
  FILE *handle;
  char **record=NULL;
  char string[MAX_LINE_LENGTH];

  handle = fopen (datin, "r");
  if (handle==NULL)
  {
    printf (" ERROR in readlist: The input file \"%s\" could not be opened.\n\n", datin);
    return;
  }
  else  printf ("\n%s opened",datin);

  lines=0;
  length=1;
  printf ("\n reading file\n");

  while (length > -1)
  {
    length = frecord( handle, string);
    if( string[length]== (char)EOF)  break;
    if((record=(char **)realloc((char **)record, (lines+1)*sizeof(char *)))==NULL)
    { printf("\n\n ERROR: realloc failed nod\n\n"); return; }
    if((record[lines]=(char *)malloc((length+1)*sizeof(char)))==NULL)
    { printf("\n\n ERROR: malloc failed nod\n\n"); return; }
    for(i=0; i<length; i++) record[lines][i]=string[i]; record[lines][i]=0;
    lines++;
  }
  fclose(handle);
  length=valuestackFlag;
  valuestackFlag=1;
  write2stack(lines, record);
  valuestackFlag=length;
  for(i=0; i<lines; i++)  free(record[i]);
  free(record);
  return;
}



int pre_read( char *record )
{
  char      datin[MAX_LINE_LENGTH];
  char      param1[MAX_LINE_LENGTH];
  char      param2[MAX_LINE_LENGTH];
  char      ext[MAX_LINE_LENGTH];
  int       e,i,j,n,l,lp,lc, length, nset, emax=0, nmax=0;
  double    gtol_buf;
  Summen    apre[1];
  Nodes     *npre=NULL;
  Elements  *epre=NULL;
  Datasets  *lpre=NULL;
  Sets      *spre=NULL;
  int   *isort=NULL;
  extern int compareInt();
  int *newnode=NULL, *newelem=NULL;
  char addDispFlagLocal=0;
  char **buf;

  datin[0]=0;
  param1[0]=0;
  param2[0]=0;
  //length = sscanf( record, "%s %s %s", datin, param1, param2 );
  length= strsplt( record, ' ', &buf);
  if(length>0) strcpy( datin, buf[0]);
  if(length>1) strcpy( param1, buf[1]);
  if(length>2) strcpy( param2, buf[2]);
  /* free buf */
  for(i=0; i<length; i++) free(buf[i]);
  free(buf);

  i=0; if( datin[0]==0) return(0); else strcpy(parameter[i++],datin);
  if( param1[0]!=0) strcpy(parameter[i++],param1);
  if( param2[0]!=0) strcpy(parameter[i++],param2);
  write2stack(length, parameter);

  /* the filetype is determined by its extension except fbd */
  /* frd: frd-format, nodes, element-numbers will be stored in a set, but are not re-defined */
  /* inp: abaqus format, nodes, elements, sets */

  /* determine the extension */
  for(i=strlen(datin); i>=0; i--) if(datin[i]=='.') break;
  for(j=++i; j<strlen(datin); j++) ext[j-i]=datin[j]; ext[j-i]=0;

  gtol_buf=gtol;
  if(compare(param1,"stack",3)==3)
  { readtostack( datin ); }
  else if(param1[0]=='-')
  { readlist( datin, &param1[1] ); }
  else if(compare( ext, "xwd", 3)== 3)
  {
    bgpicture=readxwd( datin);
    if(length==2) { bgpicture->zoom[0]=atof(param1); bgpicture->zoom[1]=atof(param1); }
    else if(length>2) { bgpicture->zoom[0]=atof(param1); bgpicture->zoom[1]=atof(param2); }
    else bgpicture->zoom[0]=bgpicture->zoom[1]=1.;
  }
  else if((compare( ext, "edg", 3)== 3)||(compare( param1, "edg", 3)== 3)) 
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    apre->n=0;
    apre->e=0;
    apre->f=0;
    apre->g=0;
    apre->emax=0;  apre->emin=MAX_INTEGER;
    apre->nmax=0;  apre->nmin=MAX_INTEGER;
    apre->l=0;
    readEdges( datin, apre, &npre, &epre );
    printf("add nodes:%d maxnr:%d\n",apre->n, apre->nmax);  

    /* modify node and element numbers if param2 == add */
    if((param2[0]=='A')||(param2[0]=='a'))
    {
      renumberfrd( anz->enext, anz->nnext, &apre[0], &npre, &epre, &lpre, &newnode, &newelem );
      free(newelem);
      free(newnode);
    }

    /* create the nodes and elements or overwrite old ones */
    for(i=0; i<apre->n; i++)  nod( anz, &node, 1, npre[i].nr, npre[npre[i].nr].nx, npre[npre[i].nr].ny, npre[npre[i].nr].nz, 1 );
    for(i=0; i<apre->e; i++) elem_define(anz,&e_enqire, epre[i].nr, epre[i].type, epre[i].nod, 1,  0 );
    printf("elements:%d maxnr:%d\n",anz->e, anz->emax);  

    free(npre);
    free(epre);

    /* new scaling for drawing */
    descalAll();
    getScaleValues( setall, set, point, node, scale);
    scalNodes ( anz->n, node, scale );
    scalPoints ( anzGeo->p, point, scale );
    scalSurfs( anzGeo->s, surf, scale);
    // recalculate the line-shapes
    for (i=0; i<anzGeo->l; i++) repLine(i);
    // recalculate the nurbl-controll-points
    for (i=0; i<anzGeo->nurl; i++) repNurl(i);
    // recalculate the nurbs-controll-points
    for (i=0; i<anzGeo->nurs; i++) repNurs(i);
    // correct the orientation of all entities
    orientSet( "all" );

    /* from here code from pre_mesh() */
    /* das neue netz muss noch zur beleuchteten ansicht aufbereitet werden  */
    makeSurfaces();
    getElemNormalen( e_enqire, node, anz->e );
    realloc_colNr();

    if(addDispFlagLocal==2) { addDispToCoordinates(node); }
  }
  else if((compare( ext, "frd", 3)== 3)&&((param1[0])&&compareStrings(param1,"new")>0))
  {
    cur_lc=cur_entity=0;
    pre_del("me");
    /* descale is needed since scale works only when a descale was done before */ 
    descalNodes ( anz->n, node, scale );
    iniMeshData( datin, "frd" );

    /* new scaling for drawing */
    descalAll();
    getScaleValues( setall, set, point, node, scale);
    scalNodes ( anz->n, node, scale );
    scalPoints ( anzGeo->p, point, scale );
    scalSurfs( anzGeo->s, surf, scale);
    // recalculate the line-shapes
    for (i=0; i<anzGeo->l; i++) repLine(i);
    // recalculate the nurbl-controll-points
    for (i=0; i<anzGeo->nurl; i++) repNurl(i);
    // recalculate the nurbs-controll-points
    for (i=0; i<anzGeo->nurs; i++) repNurs(i);
    // correct the orientation of all entities
    orientSet( "all" );

    /* calc additional entities only if the block was not jumped during read */
    for (i=0; i<anz->olc; i++)  if (lcase[i].loaded)
      calcDatasets( i, anz, node, lcase );
    /* create the mainmenu */
    if(activWindow!=-1) createDatasetEntries();
  }
  else if((compare( ext, "frd", 3)== 3)&&((param1[0])&&((compareStrings(param1,"add")<1)&&(compareStrings(param1,"ext")<1)&&(compareStrings(param1,"nom")<1))))
  {
    /* store nodes and elements in set param1 */
    printf("create set:%s\n",param1);
    readfrdfile(datin,param1);
  }
  else if((compare( ext, "frd", 3)== 3)||(compare( param1, "ng", 2)== 2)||(compare( ext, "stl", 3)== 3)||(compare( ext, "inp", 3)== 3)||(compare( param1, "inp", 3)== 3)||(compare( param1, "foam", 4)== 4)||(compare( param1, "vtk", 3)== 3)) 
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    apre->n=0;
    apre->e=0;
    apre->f=0;
    apre->g=0;
    apre->emax=0;  apre->emin=MAX_INTEGER;
    apre->nmax=0;  apre->nmin=MAX_INTEGER;
    apre->b=0;
    apre->c=0;
    apre->l=0;
    apre->sets=0;
    apre->mats=anz->mats;
    apre->amps=anz->amps;

    if(compare( ext, "frd", 3)== 3)
    {
      if( (compare( param1, "add", 3)== 3)||(compare( param2, "add", 3)== 3)||(compare( param1, "ext", 3)== 3)||(compare( param2, "ext", 3)== 3))
      {
        if(readfrd( datin, apre, &npre, &epre, &lpre, 1) <0) return(0);
      }
      else { if(readfrd( datin, apre, &npre, &epre, &lpre, read_mode) <0) return(0); }
    }
    else if(compare( param1,"foam",4)==4)
    {
      readFoam( datin, apre, &spre, &npre, &epre, &lpre );
    }
    else if(compare( ext, "stl",3)== 3)
    { 
      readStl( datin, apre, &npre, &epre, &lpre );
    }
    else if(compare( ext, "vtk",3)== 3)
    { 
      readVtk( datin, apre, &npre, &epre, &lpre );
    }
    else if(compare( param1,"ng",2)==2)
    {
      readNG( datin, apre, &spre, &npre, &epre, &lpre );
    }
    else
    {
      apre->nmax=anz->nmax;
      apre->emax=anz->emax;
      if( readccx( datin, apre, &spre, &npre, &epre, &lpre) == -1) return(0);
    }

    /* if 'nom(mesh)' is specified, scip the mesh-definition */
    if( (compare( param1, "nom", 3)== 3)||(compare( param2, "nom", 3)== 3))
    {
      if(anz->orign!=apre->n) printf("WARNING: Number of nodes:%d in the inp-file is different to the current number of nodes:%d. The meshes are potentially different.\n",apre->n,anz->orign ); 
      if(anz->e!=apre->e) printf("WARNING: Number of elements:%d in the inp-file is different to the current number of elements:%d. The meshes are potentially different.\n",apre->e,anz->e); 
    }
    else
    {
      descalAll();

      /* free the additional midside-nodes for higher order elements */
      for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
      anz->n= anz->orign;
      anz->nmax=anz->orignmax;
  
      /* modify node and element numbers if param2 == add */
      if( (compare( param1, "add", 3)== 3)||(compare( param2, "add", 3)== 3))
      {
        emax=apre->emax;
        nmax=apre->nmax;
        renumberfrd( anz->enext, anz->nnext, &apre[0], &npre, &epre, &lpre, &newelem, &newnode );
      }
      if((anz->nmin==MAX_INTEGER)||(anz->nmin>apre->nmin)) anz->nmin=apre->nmin;

      for(i=0; i<anz->n; i++) if(node[i].nr>anz->nmax) anz->nmax=node[i].nr;
      for(i=0; i<apre->n; i++) if(npre[i].nr>anz->nmax) anz->nmax=npre[i].nr;
  
      /* initialize the nodes  */
      if ((node = (Nodes *)realloc( (Nodes *)node, (anz->nmax+1)*sizeof(Nodes)) ) == NULL )
	{ errMsg("ERROR: realloc failure\n"); return(0); }
      for (i=anz->orignmax+1; i<=anz->nmax; i++) node[i].indx=-1;
      for (lc=0; lc<anz->l; lc++)
      {
        if (lcase[lc].loaded)
        {
          for(l=0; l<lcase[lc].ncomps; l++)
          {
            if ( (lcase[lc].dat[l] = (float *)realloc(lcase[lc].dat[l], (anz->nmax+1) * sizeof(float))) == NULL )
              printf("\n\n ERROR: realloc failure\n\n" );
          }
        }
      }
      /*
      if( (compare( param1, "add", 3)== 3)||(compare( param2, "add", 3)== 3))
      {
       for(i=0; i<apre->n; i++) 
       {
        n=anz->n++;        node[n].nr=npre[i].nr;
        node[node[n].nr].indx=n;
        node[node[n].nr].pflag=0;
        for (lc=0; lc<anz->l; lc++)
        {
         if (lcase[lc].loaded)
         {
          for(l=0; l<lcase[lc].ncomps; l++)
          {
            lcase[lc].dat[l][node[n].nr]=0.;
          }
         }
        }
        for (j=0; j<anz->sets; j++)
        {
          if ( set[j].flag=='o') seta( j, "n", node[n].nr );
        }
        node[node[n].nr].nx=npre[npre[i].nr].nx;
        node[node[n].nr].ny=npre[npre[i].nr].ny;
        node[node[n].nr].nz=npre[npre[i].nr].nz;
        node[node[n].nr].nv[0]=node[node[n].nr].nv[1]=node[node[n].nr].nv[2]=0.;
       }
      }
      else
      */
      {
        for(i=0; i<apre->n; i++) 
        {
          nod( anz, &node, 1, npre[i].nr, npre[npre[i].nr].nx, npre[npre[i].nr].ny, npre[npre[i].nr].nz, 0 );
        }
      }
      anz->nnext = anz->nmax+1;

      if ( (isort = (int *)realloc( isort, (anz->n+1) * sizeof(int))) == NULL )
        printf("ERROR: realloc failed: isort\n\n" ); 
      for( i=0; i<anz->n; i++) isort[i]=node[i].nr;
      qsort( isort, anz->n, sizeof(int), (void *)compareInt  );
      anz->nmax=0; anz->nmin=MAX_INTEGER;
      j=0; for (i=0; i<anz->n; i++)
      {
        if(node[isort[i]].pflag == -1) continue;
        node[j].nr = isort[i];
        node[node[j].nr].indx = j;
        if (anz->nmax<node[j].nr) anz->nmax = node[j].nr;
        if (anz->nmin>node[j].nr) anz->nmin = node[j].nr;
        j++;
      }
      anz->n=j;
      free(isort);
      
      /* generate elements */
      l=anz->emax;
      if(apre->emax>anz->emax) anz->emax=apre->emax;
      if(anz->emin==MAX_INTEGER) anz->emin=apre->emin;
      if((e_enqire=(Elements *)realloc((Elements *)e_enqire, (anz->emax+1) * sizeof(Elements))) == NULL )
        printf("\n\n ERROR: malloc failed\n\n") ;
      for (i=l+1; i<=anz->emax; i++) e_enqire[i].type=0;
      for(i=0; i<apre->e; i++)
      {
        e=elem_define(anz,&e_enqire, epre[i].nr, epre[i].type, epre[i].nod, 1, epre[i].attr );
        e_enqire[epre[i].nr].mat=epre[i].mat;
      }
      anz->enext = anz->emax+1;
  
      /* new scaling for drawing */
      getScaleValues( setall, set, point, node, scale);
      scalNodes ( anz->n, node, scale );
      scalPoints ( anzGeo->p, point, scale );
      scalSurfs( anzGeo->s, surf, scale);
      // recalculate the line-shapes
      for (i=0; i<anzGeo->l; i++) repLine(i);
      // recalculate the nurbl-controll-points
      for (i=0; i<anzGeo->nurl; i++) repNurl(i);
      // recalculate the nurbs-controll-points
      for (i=0; i<anzGeo->nurs; i++) repNurs(i);
      // correct the orientation of all entities
      orientSet( "all" );

      /* new midnodes for drawing */
      adjustDrawNodes(1);

      getElemNormalen( e_enqire, node, anz->e );
      makeSurfaces();
    }

    if(apre->l)
    {
    /* add the new datasets if no old exist */
    if(anz->l==0)
    {
      lcase=lpre;
      anz->olc=anz->l=apre->l;
      lpre=NULL;
      for(l=0; l<anz->l; l++)
      {
        if (lcase[l].loaded) calcDatasets( l, anz, node, lcase );
        if(activWindow!=-1) recompileEntitiesInMenu(l);
      }
    }
    else
    {
      /* add the new nodes to the existing datasets if option ext or add was provided, else add datasets to the existing ones */
      if( (compare( param1, "add", 3)== 3)||(compare( param2, "add", 3)== 3)||(compare( param1, "ext", 3)== 3)||(compare( param2, "ext", 3)== 3))
      {
        lp=0;
        for(l=0; l<anz->l; l++)
        {
          /* fill the datasets */
          n=strlen(lcase[l].name)-1;
          while((n>0)&&(lcase[l].name[n]==' ')) { lcase[l].name[n]=0; n--; }
          n=strlen(lpre[lp].name)-1;
          while((n>0)&&(lpre[lp].name[n]==' ')) { lpre[lp].name[n]=0; n--; }
	  printf("compare existing lcase:%s with:%s\n",lcase[l].name, lpre[lp].name); 
          if((lp<apre->l)&&(lcase[l].ncomps>=lpre[lp].ncomps)&&(compareStrings(lcase[l].name, lpre[lp].name)>0))
          {
            /* check if the data of the specified lcase (Dataset) are already available */
            if (!lcase[l].loaded)
            {
              if( pre_readfrdblock(copiedNodeSets , l, anz, node, lcase )==-1) 
              {
                printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", l+1); 
                return(0);
              }
	    }
            for(i=0; i<lcase[l].ncomps; i++)
            {
              if ( (lcase[l].dat[i] = (float *)realloc(lcase[l].dat[i], (anz->nmax+1) * sizeof(float))) == NULL )
              printf("\n\n ERROR: realloc failure nod\n\n" );
            }
            for(n=0; n<apre->n; n++) 
            {
              for(i=0; i<lpre[lp].ncomps; i++)
              {
                lcase[l].dat[i][npre[n].nr]=  lpre[lp].dat[i][npre[n].nr];
                if (lcase[l].dat[i][npre[n].nr] >  lcase[l].max[i])
                {  lcase[l].max[i]=lcase[l].dat[i][npre[n].nr]; lcase[l].nmax[i]=npre[n].nr; }
                if (lcase[l].dat[i][npre[n].nr] <  lcase[l].min[i])
                {  lcase[l].min[i]=lcase[l].dat[i][npre[n].nr]; lcase[l].nmin[i]=npre[n].nr; }
              }     
            }     
            calcDatasets( l, anz, node, lcase );
            recompileEntitiesInMenu(l);
  	    printf(" Add dataset[%d]:%s to existing dataset[%d]:%s\n",lp+1, lpre[lp].name, l+1,lcase[l].name);
            lp++;     
          }
  	  else
          {
  	    if(lp) printf(" WARNING: data from new model have different nr of components for dataset[%d]:%s and could not be merged.\n",l+1,lcase[l].name);
            if( apre->l > anz->l ) { lp++; l--; }
            if(( lp >= apre->l)||(l<0))  break;
  	  }
        }
        /* add the new datasets */
        if(!lp)
        {
          printf("additional Datasets:%d\n",apre->l);  
          if( ( lcase=(Datasets *)realloc((Datasets *)lcase,(anz->l+apre->l+1)*sizeof(Datasets))  )==NULL)
            printf("\n\n ERROR: realloc failed\n\n") ;
          for(l=0; l<apre->l; l++)
	  {
            lcase[anz->l+l]=lpre[l];
            if (lcase[anz->l+l].loaded) calcDatasets( anz->l+l, anz, node, lcase );
            if(activWindow!=-1) recompileEntitiesInMenu(anz->l+l);
	  }
          anz->l+=apre->l;
          anz->olc=anz->l;
        }
      }
      /* add the new datasets */
      else
      {
        printf("additional Datasets:%d\n",apre->l);  
        if( ( lcase=(Datasets *)realloc((Datasets *)lcase,(anz->l+apre->l+1)*sizeof(Datasets))  )==NULL)
        printf("\n\n ERROR: realloc failed\n\n") ;
        for(l=0; l<apre->l; l++)
	{
          lcase[anz->l+l]=lpre[l];
          if (lcase[anz->l+l].loaded) calcDatasets( anz->l+l, anz, node, lcase );
          if(activWindow!=-1) recompileEntitiesInMenu(anz->l+l);
	}
        anz->l+=apre->l;
        anz->olc=anz->l;
      }
    }
    }

    /* add materials and amplitudes */
    anz->mats+=apre->mats;
    anz->amps+=apre->amps;

    /* generate sets */
    for(i=0; i<apre->sets; i++)
    {
      nset=getSetNr(spre[i].name);
      // if the set does not exist then a simple copy is possible
      if(nset<0)
      {
	nset=pre_seta(spre[i].name,"i",0);
        if(newnode) for (j=0; j<spre[i].anz_n; j++) { if(spre[i].node[j]<=nmax) seta(nset,"n",newnode[spre[i].node[j]]); }
        else
	{
          if((set[nset].node= (int *)realloc((int *)set[nset].node, (spre[i].anz_n+1)*sizeof(int))) == NULL )
	    { printf("ERROR: realloc failed\n\n" ); return(0); }
	  for (j=0; j<spre[i].anz_n; j++) set[nset].node[j]=spre[i].node[j];
	  set[nset].anz_n=spre[i].anz_n;
	}
        if(newelem) for (j=0; j<spre[i].anz_e; j++) { if(spre[i].elem[j]<=emax) seta(nset,"e",newelem[spre[i].elem[j]]); }
        else
	{
          if((set[nset].elem= (int *)realloc((int *)set[nset].elem, (spre[i].anz_e+1)*sizeof(int))) == NULL )
	    { printf("ERROR: realloc failed\n\n" ); return(0); }
	  for (j=0; j<spre[i].anz_e; j++) set[nset].elem[j]=spre[i].elem[j];
	  set[nset].anz_e=spre[i].anz_e;
	}
      }
      else
      {
        if(newnode) for (j=0; j<spre[i].anz_n; j++) { if(spre[i].node[j]<=nmax) seta(nset,"n",newnode[spre[i].node[j]]); }
        else for (j=0; j<spre[i].anz_n; j++) seta(nset,"n",spre[i].node[j]);
        if(newelem) for (j=0; j<spre[i].anz_e; j++) { if(spre[i].elem[j]<=emax) seta(nset,"e",newelem[spre[i].elem[j]]); }
        else for (j=0; j<spre[i].anz_e; j++) seta(nset,"e",spre[i].elem[j]);
      }
      set[nset].type=spre[i].type;
      set[nset].material=spre[i].material;

      for (j=0; j<spre[i].anz_elf; j++)
      {
        if(newelem) e=newelem[spre[i].elf[j].e]; else e=spre[i].elf[j].e;

        /* for the moment a negative face-orientation can not be stored and is changed to positive */
        if((e_enqire[e].type>6)&&(e_enqire[e].type<11))
        {
          if(e_enqire[e].attr>3)
          {
            spre[i].elf[j].f++;
            if(e_enqire[e].type<9)
	    {
              if(spre[i].elf[j].f>4) spre[i].elf[j].f=1;
	    }
	    else
	    {
              if(spre[i].elf[j].f>5) spre[i].elf[j].f=1;
	    }
	  }
          else
          {
            spre[i].elf[j].f--;
            if(spre[i].elf[j].f==0) spre[i].elf[j].f=1;
	  }
	}

        /* store the actual face-indexes in the referenced sets */
        seta( nset, "f", face[e].indx[spre[i].elf[j].f]);
      }  
    }

    /* delete elements in special sets created by readNG */
    // (Peter)  unless keyword ndsb  (NoDeleteShellsBeams) is present
    if((compare(param1,"ng",2)==2) && (compare(param2, "ndsb",4)!=4))
    {
      for (i=0; i<anz->sets; i++)
      {
        if(set[i].name!=NULL)
	{
          if(compare(set[i].name, "+typ3", 5)==5) break;
          if(compare(set[i].name, "+typ6", 5)==5) break;
	}
      }
      if(i<anz->sets)
      {
       printf("found volume elements -> delete all shells and beams.\n");
       for (i=0; i<anz->sets; i++)
       {
        if(set[i].name!=NULL)
	{
          if(compare(set[i].name, "+typ7", 5)==5) delElem( set[i].anz_e, set[i].elem );
          if(compare(set[i].name, "+typ8", 5)==5) delElem( set[i].anz_e, set[i].elem );
          if(compare(set[i].name, "+typ11", 6)==6) delElem( set[i].anz_e, set[i].elem );
          if(compare(set[i].name, "+typ12", 6)==6) delElem( set[i].anz_e, set[i].elem );
	}
       }
       for (i=0; i<anz->sets; i++)
       {
        if((set[i].name!=NULL)&&(compare(set[i].name, "+set", 4)==4))
        {
          /* get the nodes and faces */
          completeSet(set[i].name, "do");
        }
       }
      }
    }

    realloc_colNr();

    free(newelem);
    free(newnode);

    for(j=0; j<apre->sets; j++) {  delSetx(spre[j].name); }
    free(spre); spre=NULL;
    free(npre); npre=NULL;
    free(epre); epre=NULL;
    free(lpre); lpre=NULL;
 
    /* calculate the line-shapes */
    //for (i=0; i<anzGeo->l; i++) repLine(i);
    /* calculate the nurbl-controll-points */
    //for (i=0; i<anzGeo->nurl; i++) repNurl(i);
    /* calculate the nurbs-controll-points */
    //for (i=0; i<anzGeo->nurs; i++) repNurs(i);

    if(addDispFlagLocal==2) { addDispToCoordinates(node); }
    if(activWindow!=-1) createDatasetEntries();

    if(inpformat)
    {
      updateDispLists();
      ConfigureAndShowWindow_Light();
    }
  }
  else if ((length==1)||(compare( param1, "rep", 3)== 3))
  {
    return(readfbd( datin, 0 ));
    /* updateDispLists would overwrite the elem and model edges of the 1st frame */
    //if(!animFlag) updateDispLists();
  }
  else if (compare( param1, "add", 3)== 3)
  {
    return(readfbd( datin, 1 ));
    //if(!animFlag) updateDispLists();
  } 
  else printf("ERROR, no matching file-type found\n");

  updateDispLists(); 
  createSuperSets(); 
  if(gtol_buf==gtol)
  { gtol=calcGTOL(setall);  printf ("gtol calculated:%e\n", gtol); }

  printf (" ready\n");
  return(0);
}


/* combine single sets to a superSet */
void createSuperSets(void)
{
  int i,j,s,flmSet=-1, radSet=-1, cflxSet=-1, dflxSet=-1;

  s=anz->sets;
  for (i=0; i<s; i++) if(set[i].name!=(char *)NULL)
  {
    /* create additional special sets which add sets of a ceratin type */
    if(compare(set[i].name,"+flm",4)==4)
    {
      if(flmSet<0) flmSet=pre_seta("+flm","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(flmSet,"f",set[i].face[j]);
    }
    if(compare(set[i].name,"+rad",4)==4)
    {
      if(radSet<0) radSet=pre_seta("+rad","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(radSet,"f",set[i].face[j]);
    }
    if(compare(set[i].name,"+cflx",5)==5)
    {
      if(cflxSet<0) cflxSet=pre_seta("+cflx","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(cflxSet,"f",set[i].face[j]);
    }
    if(compare(set[i].name,"+dflx",5)==5)
    {
      if(dflxSet<0) dflxSet=pre_seta("+dflx","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(dflxSet,"f",set[i].face[j]);
    }
  }
}



/* define or change the element-typ in the struct of the lines, surfs, bodies */
void pre_elty( char *record )
{
  int  i,j, length, setNr;
  char setname[MAX_LINE_LENGTH], etype[MAX_LINE_LENGTH], eparm[MAX_LINE_LENGTH];
  static int elty_callNr=0;
  char attr;
  char elty[MAX_LINE_LENGTH];
  char **elty_string=NULL;

  setname[0]=etype[0]=eparm[0]='\0';
  length= sscanf( record, "%s%s%s", setname, etype, eparm);
  if(compareStrings(setname, "ALL")==3) strcpy(setname,"all");
  autoEltyFlag=0;

  if(length<1)
  {
    /* show all sets which have an element type assigned */
    /* search the highest set[i].eseq (number of elty commands) */
    j=1; for(i=0; i<anz->sets; i++)
    {
      if(set[i].name!=(char *)NULL) j=imax(j, set[i].eseq);
    }
    j++;
    if((elty_string=(char **)malloc(j*sizeof(char *))) == NULL) printf("\n\n ERROR:malloc failure\n\n");
    for (i=0; i<j; i++) elty_string[i]=(char *)NULL;

    for (i=0; i<anz->sets; i++)
    {
      if(set[i].name!=(char *)NULL)
      {
        if(set[i].eattr==1)  attr='R';
        else if(set[i].eattr==2)  attr='I';
        else if(set[i].eattr==3)  attr='D';
        else if(set[i].eattr==4)  attr='E';
        else if(set[i].eattr==5)  attr='S';
        else if(set[i].eattr==6)  attr='C';
        else if(set[i].eattr==7)  attr='F';
        else if(set[i].eattr==8)  attr='M';
        else if(set[i].eattr==9)  attr='T';
        else if(set[i].eattr==-1)  attr='U';
        else attr=' ';
        if(set[i].etyp==1) sprintf (elty, " ELTY %s HE8%c", set[i].name, attr);
        if(set[i].etyp==3) sprintf (elty, " ELTY %s TE4%c", set[i].name, attr);
        if(set[i].etyp==4) sprintf (elty, " ELTY %s HE20%c", set[i].name, attr);
        if(set[i].etyp==6) sprintf (elty, " ELTY %s TE10%c", set[i].name, attr);
        if(set[i].etyp==7) sprintf (elty, " ELTY %s TR3%c", set[i].name, attr);
        if(set[i].etyp==8) sprintf (elty, " ELTY %s TR6%c", set[i].name, attr);
        if(set[i].etyp==9) sprintf (elty, " ELTY %s QU4%c", set[i].name, attr);
        if(set[i].etyp==10) sprintf (elty, " ELTY %s QU8%c", set[i].name, attr);
        if(set[i].etyp==11) sprintf (elty, " ELTY %s BE2%c", set[i].name, attr);
        if(set[i].etyp==12) sprintf (elty, " ELTY %s BE3%c", set[i].name, attr);
        if(set[i].etyp>0)
	{
          if((elty_string[set[i].eseq]=(char *)malloc(MAX_LINE_LENGTH*sizeof(char))) == NULL) printf("\n\n ERROR:malloc failure\n\n");

          if(set[i].eparm!=(char *)NULL) sprintf(elty_string[set[i].eseq], "%s %s",elty, set[i].eparm);
          else sprintf(elty_string[set[i].eseq], "%s",elty);
        }
      }
    }
    for (i=0; i<j; i++)
    {
      if(elty_string[i]!=(char *)NULL)
      {
        printf("%s\n", elty_string[i]);
        free(elty_string[i]);
      }
    }
    free(elty_string);
  }
  else if(length==1)
  {
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" ERROR in pre_elty: set:%s does not exist\n", setname);
      return;
    }

    /* delete all element definitions if the setname is "all" */
    if(compareStrings(setname, "all")>0)
    {
      for (setNr=0; setNr<anz->sets; setNr++)
      {
        set[setNr].etyp=0;
        set[setNr].eattr=0;
        if(set[setNr].eparm!=(char *)NULL)
	{
          free(set[setNr].eparm);
          set[setNr].eparm=(char *)NULL;
	}
        for (i=0; i<set[setNr].anz_b; i++) if(!body[set[setNr].body[i]].elock)
        {
          body[set[setNr].body[i]].eattr=set[setNr].eattr ;
          body[set[setNr].body[i]].etyp=set[setNr].etyp ;
          if(body[set[setNr].body[i]].eparm!=(char *)NULL)
	  { free(body[set[setNr].body[i]].eparm) ; body[set[setNr].body[i]].eparm=(char *)NULL; }
        }
        for (i=0; i<set[setNr].anz_s; i++) if(!surf[set[setNr].surf[i]].elock)
        {
          surf[set[setNr].surf[i]].eattr=set[setNr].eattr ;
          surf[set[setNr].surf[i]].etyp=set[setNr].etyp ;
          if(surf[set[setNr].surf[i]].eparm!=(char *)NULL)
	  { free(surf[set[setNr].surf[i]].eparm) ; surf[set[setNr].surf[i]].eparm=(char *)NULL; }
        }
        for (i=0; i<set[setNr].anz_l; i++) if(!line[set[setNr].line[i]].elock)
        {
          line[set[setNr].line[i]].eattr=set[setNr].eattr ;
          line[set[setNr].line[i]].etyp=set[setNr].etyp ;
        }
      }
    }
    /* delete only the element-def of this set */
    else
    {
      set[setNr].etyp=0;
      set[setNr].eattr=0;
      set[setNr].eparm=(char *)NULL;
      if(set[setNr].eparm!=(char *)NULL)
      {
        free(set[setNr].eparm);
        set[setNr].eparm=(char *)NULL;
      }
      for (i=0; i<set[setNr].anz_b; i++) if(!body[set[setNr].body[i]].elock)
      {
        body[set[setNr].body[i]].eattr=set[setNr].eattr ;
        body[set[setNr].body[i]].etyp=set[setNr].etyp ;
        if(body[set[setNr].body[i]].eparm!=(char *)NULL)
        { free(body[set[setNr].body[i]].eparm) ; body[set[setNr].body[i]].eparm=(char *)NULL; }
      }
      for (i=0; i<set[setNr].anz_s; i++) if(!surf[set[setNr].surf[i]].elock)
      {
        surf[set[setNr].surf[i]].eattr=set[setNr].eattr ;
        surf[set[setNr].surf[i]].etyp=set[setNr].etyp ;
        if(surf[set[setNr].surf[i]].eparm!=(char *)NULL)
        { free(surf[set[setNr].surf[i]].eparm) ; surf[set[setNr].surf[i]].eparm=(char *)NULL; }
      }
      for (i=0; i<set[setNr].anz_l; i++) if(!line[set[setNr].line[i]].elock)
      {
        line[set[setNr].line[i]].eattr=set[setNr].eattr ;
        line[set[setNr].line[i]].etyp=set[setNr].etyp ;
      }
    }
  }
  else
  {
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" ERROR in pre_elty: set:%s does not exist\n", setname);
      return;
    }
    for(i=0;i<strlen(etype); i++) etype[i]=toupper(etype[i]);
    for(i=0;i<strlen(eparm); i++) eparm[i]=toupper(eparm[i]);

    if( compare( etype, "LOC", 3) == 3)
    {
      for (i=0; i<set[setNr].anz_b; i++) body[set[setNr].body[i]].elock=1;
      for (i=0; i<set[setNr].anz_s; i++) surf[set[setNr].surf[i]].elock=1;
      for (i=0; i<set[setNr].anz_l; i++) line[set[setNr].line[i]].elock=1;
      return;
    }
    if( compare( etype, "ULO", 3) == 3)
    {
      for (i=0; i<set[setNr].anz_b; i++) body[set[setNr].body[i]].elock=0;
      for (i=0; i<set[setNr].anz_s; i++) surf[set[setNr].surf[i]].elock=0;
      for (i=0; i<set[setNr].anz_l; i++) line[set[setNr].line[i]].elock=0;
      return;
    }
    set[setNr].eseq=elty_callNr++;

    if(strlen(eparm))
    {
      if((set[setNr].eparm= (char *)realloc((char *)set[setNr].eparm, (strlen(eparm)+1)*sizeof(char))) == NULL )
      { printf("ERROR: malloc failed in seta()\n\n" ); return; }
      strcpy(set[setNr].eparm, eparm);
    }
    else { free(set[setNr].eparm); set[setNr].eparm=(char *)NULL; } 

    if( compare( etype, "BE2", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=11; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=11; set[setNr].eattr=1; }
      else if(etype[3]=='D') { set[setNr].etyp=11; set[setNr].eattr=3; }
      else if(etype[3]=='F') { set[setNr].etyp=11; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "BE3", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=12; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=12; set[setNr].eattr=1; }
      else if(etype[3]=='F') { set[setNr].etyp=12; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TR3", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=7; set[setNr].eattr=0; }
      else if(etype[3]=='U') { set[setNr].etyp=7; set[setNr].eattr=-1; }
      else if(etype[3]=='E') { set[setNr].etyp=7; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=7; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=7; set[setNr].eattr=6; }
      else if(etype[3]=='F') { set[setNr].etyp=7; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TR6", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=8; set[setNr].eattr=0; }
      else if(etype[3]=='U') { set[setNr].etyp=8; set[setNr].eattr=-1; }
      else if(etype[3]=='E') { set[setNr].etyp=8; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=8; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=8; set[setNr].eattr=6; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "QU4", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=9; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=9; set[setNr].eattr=1; }
      else if(etype[3]=='E') { set[setNr].etyp=9; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=9; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=9; set[setNr].eattr=6; }
      else if(etype[3]=='F') { set[setNr].etyp=9; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);

      if((strlen(etype)==5)&&(etype[4]=='R')) { set[setNr].eattr+=10; }
    }
    else if(compare( etype, "QU8", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=10; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=10; set[setNr].eattr=1; }
      else if(etype[3]=='E') { set[setNr].etyp=10; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=10; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=10; set[setNr].eattr=6; }
      else printf("ERROR: type %s not known\n", etype);

      if((strlen(etype)==5)&&(etype[4]=='R')) { set[setNr].eattr+=10; }
    }
    else if(compare( etype, "HE8", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=1; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=1; set[setNr].eattr=1; }
      else if(etype[3]=='I') { set[setNr].etyp=1; set[setNr].eattr=2; }
      else if(etype[3]=='F') { set[setNr].etyp=1; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "HE20", 4) == 4)
    {
      if(strlen(etype)==4)   { set[setNr].etyp=4; set[setNr].eattr=0; }
      else if(etype[4]=='R') { set[setNr].etyp=4; set[setNr].eattr=1; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "PE6", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=2; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=2; set[setNr].eattr=1; }
      else if(etype[3]=='I') { set[setNr].etyp=2; set[setNr].eattr=2; }
      else if(etype[3]=='F') { set[setNr].etyp=2; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "PE15", 4) == 4)
    {
      if(strlen(etype)==4)   { set[setNr].etyp=5; set[setNr].eattr=0; }
      else if(etype[4]=='R') { set[setNr].etyp=5; set[setNr].eattr=1; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TE4", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=3; set[setNr].eattr=0; }
      else if(etype[3]=='F') { set[setNr].etyp=3; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TE10", 4) == 4)
    {
      if(strlen(etype)==4)   { set[setNr].etyp=6; set[setNr].eattr=0; }
      else if(etype[4]=='M') { set[setNr].etyp=6; set[setNr].eattr=8; }
      else if(etype[4]=='T') { set[setNr].etyp=6; set[setNr].eattr=9; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else 
    {
      printf("ERROR: element-type:%s not known (choose either be2,be3,tr3,tr6,qu4,qu8,he8,he20,te4,te10)\n",etype);
      return;
    }
    
    /* assign the element-type to the appropriate entities */ 
    /* assign the attribute */
    /* get the basis formulation of the etyp */
    if(set[setNr].etyp<7)
    {
      for (i=0; i<set[setNr].anz_b; i++)
      {
       if(!body[set[setNr].body[i]].elock)
       {
        body[set[setNr].body[i]].eattr=set[setNr].eattr ;
        body[set[setNr].body[i]].etyp=set[setNr].etyp ;
        if(set[setNr].eparm!=(char *)NULL) {
        if((body[set[setNr].body[i]].eparm= (char *)realloc((char *)body[set[setNr].body[i]].eparm, (strlen(set[setNr].eparm)+1)*sizeof(char))) == NULL )
        { printf("ERROR: malloc failed in prnt()\n\n" ); return; }
        strcpy(body[set[setNr].body[i]].eparm,set[setNr].eparm); }
       }
       else printf(" body %s locked\n", body[set[setNr].body[i]].name);
      }
    }
    if((set[setNr].etyp>=7)&&(set[setNr].etyp<=10))
    {
      for (i=0; i<set[setNr].anz_s; i++)
      {
       if(!surf[set[setNr].surf[i]].elock)
       {
        surf[set[setNr].surf[i]].eattr=set[setNr].eattr ;
        surf[set[setNr].surf[i]].etyp=set[setNr].etyp ;
        if(set[setNr].eparm!=(char *)NULL)
        {
          if((surf[set[setNr].surf[i]].eparm= (char *)realloc((char *)surf[set[setNr].surf[i]].eparm, (strlen(set[setNr].eparm)+1)*sizeof(char))) == NULL )
          { printf("ERROR: malloc failed in prnt()\n\n" ); return; }
          strcpy(surf[set[setNr].surf[i]].eparm,set[setNr].eparm);
        }
        else { free(surf[set[setNr].surf[i]].eparm); surf[set[setNr].surf[i]].eparm=(char *)NULL; }
       }
       else printf(" surf %s locked\n", surf[set[setNr].surf[i]].name);
      }
    }
    if((set[setNr].etyp>=11)&&(set[setNr].etyp<=12))
    {
      for (i=0; i<set[setNr].anz_l; i++)
      {
       if(!line[set[setNr].line[i]].elock)
       {
        line[set[setNr].line[i]].eattr=set[setNr].eattr ;
        line[set[setNr].line[i]].etyp=set[setNr].etyp ;
       }
       else printf(" line %s locked\n", line[set[setNr].line[i]].name);
      }
    }
    for (i=0; i<set[setNr].anz_e; i++)
    {
      if(e_enqire[set[setNr].elem[i]].type==set[setNr].etyp) e_enqire[set[setNr].elem[i]].attr=set[setNr].eattr ;
    }
  }
}



/* define the threshold-values for element criterions */
void pre_eqal( char *record )
{
  int j;
  char  etest[MAX_LINE_LENGTH];
  double evalue;

  etest[0]='\0';
  sscanf( record, "%s%lf", etest, &evalue);
  for(j=0;j<=strlen(etest); j++) etest[j]=toupper(etest[j]);

  if(compareStrings(etest,"JBIR")>0)
  {
    eqal.jbir=evalue;
  }
  else if(compareStrings(etest,"ASPR")>0)
  {
    eqal.aspr=evalue;
  }
  else if(compareStrings(etest,"MCA")>0)
  {
    eqal.mca=evalue;
  }
  else
  {
    printf(" current element quality thresholds (0==off):\n JBIR:%lf\n ASPR:%lf\n MCA:%lf\n", eqal.jbir, eqal.aspr, eqal.mca);
    sprintf(parameter[0],"%lf",eqal.jbir);
    sprintf(parameter[1],"%lf",eqal.aspr);
    sprintf(parameter[2],"%lf",eqal.mca);
    write2stack(3, parameter);
  }
}


/* store the entities to be meshed in a special set and add lower entities, then mesh   */

int pre_mesh( char *record )
{
  int c,l,n,s,i,j,k,se, tetSet=0;
  char buffer[MAX_LINE_LENGTH],setname[MAX_LINE_LENGTH], code[4][MAX_LINE_LENGTH];
  int   anz_nmax,length, setNr, sets, meshoptFlag_length=MESHOPT_LENGTH, meshoptFlag_angle=MESHOPT_ANGLE, blockFlag=0, lonlyFlag=0, projFlag=1, tetFlag=0;
  double teth=1e6;
  int ntmp,etmp=0,eSet;

  length = sscanf( record,"%s %s %s %s %s", setname,code[0],code[1],code[2],code[3] );

  for(i=0; i<length-1; i++)
  {
    if(compare(code[i],"nolength",4)==4)  meshoptFlag_length=0;
    else if(compare(code[i],"noangle",4)==4)  meshoptFlag_angle=0;
    else if(compare(code[i],"length",4)==4)  meshoptFlag_length=1;
    else if(compare(code[i],"angle",4)==4)  meshoptFlag_angle=1;
    else if(compare(code[i],"block",3)==3) blockFlag=1;
    else if(compare(code[i],"fast",3)==3) projFlag=0;
    else if(compare(code[i],"lonly",3)==3) lonlyFlag=1;
    /* a rudimentary implementation of netgen to generate tets */
    else if(compare(code[i],"tet",3)==3)
    {
      tetFlag=1;
      if(length == i+3) teth=(double)atof(code[i+1]);
    }
  }

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in pre_mesh: set:%s does not exist\n", setname);
    return(-1);
  }
  else
    if(printFlag) printf (" set[%d]:%s will be meshed\n", setNr, setname);

  printf (" please wait for 'ready'\n");
  setall=getSetNr("all");

  /* cycle through all entities and add them to the special set  */
  
  if( (se=pre_seta( specialset->mesh, "i", 0)) <0 ) return(-1);
  if(setNr==setall)
  {
    for (i=0; i<anz->n; i++) if(node[node[i].nr].pflag==0) seta( se, "n", node[i].nr );
    for (i=0; i<anz->e; i++) seta( se, "e", e_enqire[i].nr );
    for (i=0; i<anzGeo->l; i++) if( line[i].name != (char *)NULL ) seta( se, "l", i );
    for (i=0; i<anzGeo->p; i++) if( point[i].name != (char *)NULL ) seta( se, "p", i );
    for (i=0; i<anzGeo->l; i++) if( line[i].name != (char *)NULL ) seta( se, "l", i );
    for (i=0; i<anzGeo->s; i++) if( surf[i].name != (char *)NULL ) { seta( se, "s", i ); surf[i].permElem=1; }
    for (i=0; i<anzGeo->b; i++) if( body[i].name != (char *)NULL ) seta( se, "b", i );
  }
  else 
  {
    for (i=0; i<anzGeo->s; i++) if( surf[i].name != (char *)NULL ) { surf[i].permElem=0; }
    
    for (i=0; i<set[setNr].anz_n; i++) seta( se, "n", set[setNr].node[i] );
    for (i=0; i<set[setNr].anz_e; i++) seta( se, "e", set[setNr].elem[i] );
    for (i=0; i<set[setNr].anz_b; i++) seta( se, "b", set[setNr].body[i] );
    for (i=0; i<set[setNr].anz_s; i++) { seta( se, "s", set[setNr].surf[i] ); surf[set[setNr].surf[i]].permElem=1; }
    for (i=0; i<set[setNr].anz_c; i++) seta( se, "c", set[setNr].lcmb[i] );
    for (i=0; i<set[setNr].anz_l; i++) seta( se, "l", set[setNr].line[i] );
    /* second cycle through all entities and add lower ones  to the special set  */
    /* completeSet( specialset->mesh, "do") ; */

    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[se].anz_b; i++)
    {
      c= set[se].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( se, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs and nurbs */
    for (i=0; i<set[se].anz_s; i++)
    {
      s= set[se].surf[i];
      //if(surf[s].sh>-1) seta( se, "sh", surf[s].sh );
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( se, "l", l );
        }
        else
        {
          seta( se, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[se].anz_c; i++)
    {
      c= set[se].lcmb[i];
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( se, "l", l );
      }
    }
    /* cyrcle through all lines and add all points, trks */
    for (i=0; i<set[se].anz_l; i++)
    {
      l= set[se].line[i];
      seta( se, "p", line[l].p1 );
      seta( se, "p", line[l].p2 );
      if (line[l].typ=='a') seta( se, "p", line[l].trk );
      if (line[l].typ=='s') seta( se, "se", line[l].trk );
    }
    /* cyrcle through all nurbs and add all points */
    /*
    for (i=0; i<set[se].anz_nurs; i++)
    {
      s= set[se].nurs[i];
      for (j=0; j<nurbs[s].u_npnt; j++)
        for (k=0; k<nurbs[s].v_npnt; k++)
          seta( se, "p", nurbs[s].ctlpnt[j][k] );
    }
    */
  }

  /* cyrcle through all bodys and check the body-etyp, for tet-requests set the surface-etyp */
  for (i=0; i<set[se].anz_b; i++)
  {
    c= set[se].body[i];
    if(body[c].etyp==3)
    {
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        //if(surf[l].etyp!=7) { surf[l].etyp=7; surf[l].eattr=-1; }
        if((surf[l].etyp!=7)&&(!surf[l].elock)) { surf[l].etyp=7; surf[l].eattr=-1; }
      }
    }
    if(body[c].etyp==6)
    {
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        //if(surf[l].etyp!=8) { surf[l].etyp=8; surf[l].eattr=-1; }
        if((surf[l].etyp!=8)&&(!surf[l].elock)) { surf[l].etyp=8; surf[l].eattr=-1; }
      }
    }
  }

  /* cyrcle again through all bodys and check the body-etyp for hex-requests, then eventually change the eattr of the surfaces */
  /* do it even if surf[nr].elock=1 since the hex needs regular node pattern and since an evantual neighboring tet mesh can use that also */
  for (i=0; i<set[se].anz_b; i++)
  {
    c= set[se].body[i];
    if((body[c].etyp==1)||(body[c].etyp==4))
    {
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        if(surf[l].eattr==-1) { surf[l].elock=0; surf[l].eattr=0; }
      }
    }
  }
  
  orientSet( specialset->mesh );

  /* free the additional midside-nodes for higher order elements */
  for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
  anz->n= anz->orign;
  anz->nmax=anz->orignmax;

  anz_nmax=anz->nmax;
  /* mesh an existing surface-mesh */
  if(tetFlag)
  {
    if(set[setNr].flag!='o') { i=1; seto(set[setNr].name); } else { i=0; }
    generateTetFromSet(se, teth, set[se].eattr, meshp.tetmesher);
    if(i) setc(set[setNr].name);
    adjustDrawNodes(1);
  }
  else
  {
    /* mesh the geometry */
    if(meshSet( specialset->mesh, blockFlag, lonlyFlag, projFlag, meshoptFlag_length, meshoptFlag_angle) <1) goto scipMesh; 
    setall=getSetNr("all");
    //elemChecker( set[setall].anz_e, set[setall].elem, node, e_enqire);
    adjustDrawNodes(1);

    /* improve bad tr3 elements */
    improveBadTr3(specialset->mesh);

    /* cyrcle again through all bodys and check if mpcs have to be generated to glue incompatible element-formulations (he8+he20, tet+he20, etc) */
    for (i=0; i<set[se].anz_b; i++)
    {
      c= set[se].body[i];
      if(body[c].etyp==4)
      {
        for (j=0; j<body[c].ns; j++)
        {
          l=body[c].s[j];
  
          /* if surf[l].etyp is defined then a connection of a mid-face node and the edges has to be prepared (mpc) */
          if(((surf[l].etyp==7)||(surf[l].etyp==8))&&(body[c].etyp==4))
  	  {
            /* generate a set with all nodes and related volume elements of the surface */
            ntmp=pre_seta("-ntmp","s",surf[l].name) ;
            etmp=pre_seta("-etmp","i",0) ;
            for(k=0; k<set[ntmp].anz_n; k++)  seta(etmp,"n",set[ntmp].node[k]);
            completeSet("-etmp","up");
            eSet=pre_seta("-eSet","i",0) ;
            for(k=0; k<set[etmp].anz_e; k++) if(e_enqire[set[etmp].elem[k]].type == 4) seta(eSet,"e",set[etmp].elem[k]);
  	    completeSet("-eSet","do");
            completeSet("-ntmp","do");
  
            /* create dep and indepsets */
            if ( (depSet = (int *)realloc( depSet, (sum_equSets+1) * sizeof(int))) == NULL )
              printf("ERROR: realloc failed: meshSet\n\n" );
            if ( (indSet = (int *)realloc( indSet, (sum_equSets+1) * sizeof(int))) == NULL )
              printf("ERROR: realloc failed: meshSet\n\n" );
            sprintf(buffer,"-depSet_tmp%d", sum_equSets);
            depSet[sum_equSets]=pre_seta(buffer,"i",0) ;
            for(k=0; k<set[ntmp].anz_n; k++)  seta(depSet[sum_equSets],"n",set[ntmp].node[k]);
            for(k=0; k<set[eSet].anz_n; k++)  setr(depSet[sum_equSets],"n",set[eSet].node[k]);
            sprintf(buffer,"-indSet_tmp%d", sum_equSets);
            indSet[sum_equSets]=pre_seta(buffer,"i",0) ;
            for(k=0; k<set[ntmp].anz_n; k++)  seta(indSet[sum_equSets],"n",set[ntmp].node[k]);
            for(k=0; k<set[depSet[sum_equSets]].anz_n; k++)  setr(indSet[sum_equSets],"n",set[depSet[sum_equSets]].node[k]);
            for(k=0; k<set[eSet].anz_e; k++)  seta(indSet[sum_equSets],"e",set[eSet].elem[k]);
            sum_equSets++;

            /* delete the temporary sets */
            delSet(set[ntmp].name);
            delSet(set[etmp].name);
            delSet(set[eSet].name);
 	  }
        }
      }
    }

    /* tet-mesh */
    etmp=0;
    tetSet=-1;
    for (i=0; i<set[se].anz_b; i++)
    {
      c= set[se].body[i];
      if((body[c].etyp==3)||(body[c].etyp==6))
      {
        if(tetSet==-1)
	{
          delSet( "+tetSet" );
          tetSet=pre_seta("+tetSet","i",0);
	}

        /* the final orientation of nurbs-surfaces for the 2D-mesh requires a call to orientBody() */
        orientBody(c);

        /* get the surface-elements for later deletion */
        for(j=0; j<body[c].ns; j++)
	{
          s=body[c].s[j];
          for(k=0; k<surf[s].ne; k++) seta(tetSet,"e",surf[s].elem[k]);
	}

	printf("\n Tet-mesh body:%s\n\n", body[c].name);
        if(body[c].eparm!=(char *)NULL) etmp=generateTetFromBody(c, (double)atof(body[c].eparm), body[c].eattr, meshp.tetmesher);
        else etmp=generateTetFromBody(c, (double)1.e6, body[c].eattr, meshp.tetmesher);
        adjustDrawNodes(1);

        if(etmp>0)
	{
	  printf("\n %d Tet-elems in body:%s\n\n", body[c].ne, body[c].name);
 
          /* add the inner nodes and elements to the sets */
          for(j=0; j<anz->sets; j++)
          {
            if((set[j].name!=(char *)NULL)&&(set[j].name[0]!='-')&&( getIndex(&set[j].body,set[j].anz_b, c) >-1))
  	    {
              for(n=0; n<body[c].nn; n++) seta(j,"n", body[c].nod[n]);
              for(n=0; n<body[c].ne; n++) seta(j,"e", body[c].elem[n]);
  	    }
          }
	}
        else
	{
          printf(" ERROR: No tet-mesh could be created for body:%s\n ", body[c].name);
          printf(" The temporary surface mesh is not deleted and can be used for debugging.\n");
          printf(" The files nodnr_ng_cgx.out elemnr_ng_cgx.out provide the link between ng and cgx entity numbers.\n");
          break;
	}
      }
    }
  scipMesh:;
    /* delete the surface-elements which were used to create tets */
    if(etmp>0) zap( "+tetSet" );
  }

  /* in case that datasets had already existed then this fields have to be extended */
  if (anz->l > 0)
  {
    for (l=0; l< anz->l; l++)
    {
      if (!lcase[l].loaded)
      {
        if( pre_readfrdblock(copiedNodeSets , l, anz, node, lcase )==-1) 
        {
          printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", l+1); 
          return(-1);
        }
        calcDatasets( l, anz, node, lcase );
        recompileEntitiesInMenu(l);
      }
      for (i=0; i<lcase[l].ncomps; i++)
      {
        if ( (lcase[l].dat[i] = (float *)realloc( lcase[l].dat[i], (anz->nmax+2) * sizeof(float))) == NULL )
          printf("\n\n ERROR: realloc failure\n\n" );	               
        for(j=anz_nmax+1; j<=anz->nmax; j++) lcase[l].dat[i][j]=0.;
      }
    }
  }
  //anz->olc       = anz->l;

  if (calcBadElements("all")>0)
  {
    printf(" try to fix bad elements\n");
    fixMidsideNodes( specialset->njby, "lin");
    printf(" calcBadElements\n");
    calcBadElements("all");
  }

  printf(" makeSurfaces\n");
  makeSurfaces();
  printf(" getElemNormalen\n");
  getElemNormalen( e_enqire, node, anz->e );
  printf(" realloc_colNr\n");
  realloc_colNr();

  printf(" add the faces\n");
  /* add the faces */
  for (sets=0; sets<anz->sets; sets++)
  {
    if ((set[sets].type==0)&&(set[sets].name!=(char *)NULL))
    { 
      //set[sets].anz_f=0;
      completeSet( set[sets].name, "f" );
    }
  }

  printf(" updateDispLists\n");
  updateDispLists(); 

  /* clear special set  */
  printf(" delSet\n");
  delSet(specialset->mesh );

  // dirty hack: the glNormal in drawElements_plot() crashes the threading when meshing a second set
  if(anz->threads>1) for (j=0; j<anzGeo->psets; j++ ) if(pset[j].type[0]=='e') pset[j].type[0]='w';
  
  printf (" ready\n");
  return(setNr);
}



int completeSet_Faces( int setNr, int setNrbuf, int *fUsed, int flag)
{
  int i,j;
  int m, n, ipuf=0 ;

  if(!flag)
  {
   if (set[setNr].anz_n>0)
   {
    /* suche abhaengige faces */
    m=set[setNrbuf].anz_f;
    for ( i=0; i<anz->f; i++)
    {
      switch(face[i].type)
      {
        case 7:
          ipuf = 3;  /* TRI3  */
        break;
        case 8:
          //ipuf = 6;  /* TRI6  */
          ipuf = 3;  /* TRI6  */
        break;
        case 9:
          ipuf = 4;  /* QUAD4 */
        break;
        case 10:
          //ipuf = 8; /* QUAD8 */
          ipuf = 4; /* QUAD8 */
        break;
        case 11:
          ipuf = 2; /* BEAM */
        break;
        case 12:
          //ipuf = 3; /* BEAM3 */
          ipuf = 2; /* BEAM3 */
        break;
      }
      for (j=0; j<ipuf; j++)
      {
        if(ifind(&set[setNr].node, set[setNr].anz_n, face[i].nod[j])>-1)
        {
          //printf("n:%d f:%d used:%d\n", face[i].nod[j], face[i].nr, fUsed[i]);
          if( !fUsed[i] )
          {
            if ( (set[setNrbuf].face = (int *)realloc((int *)set[setNrbuf].face, (set[setNrbuf].anz_f+1) * sizeof(int))) == NULL )
              printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNrbuf].name);
            set[setNrbuf].face[set[setNrbuf].anz_f]= i;
            set[setNrbuf].anz_f++;
          }
          break;
        }
      }
    }
    if(set[setNrbuf].anz_f-m)
    {
      qsort( set[setNrbuf].face, set[setNrbuf].anz_f, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNrbuf].anz_f; i++)
      {
	if(set[setNrbuf].face[n]!=set[setNrbuf].face[i]) set[setNrbuf].face[++n]=set[setNrbuf].face[i];
      }
      set[setNrbuf].anz_f=n+1;
    }

   }
  }
  else
  {
    /* circle through all faces and add all nodes */
    m=set[setNr].anz_n;
    for (n=0; n<set[setNr].anz_f; n++)
    {
      i=set[setNr].face[n];
      switch(face[i].type)
      {
        case 7:
          ipuf = 3;  /* TRI3  */
        break;
        case 8:
          ipuf = 6;  /* TRI6  */
        break;
        case 9:
          ipuf = 4;  /* QUAD4 */
        break;
        case 10:
          ipuf = 8; /* QUAD8 */
        break;
        case 11:
          ipuf = 2; /* BEAM */
        break;
        case 12:
          ipuf = 3; /* BEAM3 */
        break;
      }
      for (j=0; j<ipuf; j++)
      {
          if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
            printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
          set[setNr].node[set[setNr].anz_n]= face[i].nod[j];
          set[setNr].anz_n++;
          //printf("n:%d f:%d used:%d\n", face[i].nod[j], i, fUsed[i]);
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
	//if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[n++]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
      //set[setNr].anz_n=n;
    }
  }
  return(1);
}



int completeSet_Mesh( int setNr, int setNrbuf, int *elUsed, int flag)
{
  int i,j;
  int m, n, ipuf=0 ;

  if(!flag)
  {
   if (set[setNr].anz_n>0)
   {
    /* suche abhaengige elemente */
    m=set[setNrbuf].anz_e;
    for ( i=0; i<anz->e; i++)
    {
      switch(e_enqire[e_enqire[i].nr].type)
      {
        case 1:
          ipuf = 8;       /* HEXA8 */
        break;
        case 2:
          ipuf = 6;  /* PENTA6 */
        break;
        case 3:
          ipuf = 4;  /* TET4 */
        break;
        case 4:
          //ipuf = 20; /* HEXA20 */
          ipuf = 8; /* HEXA20 */
        break;
        case 5:
          ipuf = 15; /* PENTA15 */
        break;
        case 6:
          //ipuf = 10; /* TET10 */
          ipuf = 4; /* TET10 */
        break;
        case 7:
          ipuf = 3;  /* TRI3  */
        break;
        case 8:
	  // ipuf = 6;  /* TRI6  */
          ipuf = 3;  /* TRI6  */
        break;
        case 9:
          ipuf = 4;  /* QUAD4 */
        break;
        case 10:
          //ipuf = 8; /* QUAD8 */
          ipuf = 4; /* QUAD8 */
        break;
        case 11:
          ipuf = 2; /* BEAM */
        break;
        case 12:
          //ipuf = 3; /* BEAM3 */
          ipuf = 2; /* BEAM3 */
        break;
      }
      for (j=0; j<ipuf; j++)
      {
        if(ifind(&set[setNr].node, set[setNr].anz_n, e_enqire[e_enqire[i].nr].nod[j])>-1)
        {
          //printf("    e:%d \n", e_enqire[i].nr);
          if( !elUsed[e_enqire[i].nr] )
          {
            //printf("add e:%d \n", e_enqire[i].nr);
            //seta( setNrbuf, "e", e_enqire[i].nr );
            if ( (set[setNrbuf].elem = (int *)realloc((int *)set[setNrbuf].elem, (set[setNrbuf].anz_e+1) * sizeof(int))) == NULL )
              printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNrbuf].name);
            set[setNrbuf].elem[set[setNrbuf].anz_e]= e_enqire[i].nr;
            set[setNrbuf].anz_e++;
          }
          break;
        }
      }
    }
    if(set[setNrbuf].anz_e-m)
    {
      qsort( set[setNrbuf].elem, set[setNrbuf].anz_e, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNrbuf].anz_e; i++)
      {
	if(set[setNrbuf].elem[n]!=set[setNrbuf].elem[i]) set[setNrbuf].elem[++n]=set[setNrbuf].elem[i];
      }
      set[setNrbuf].anz_e=n+1;
    }

   }
  }
  else
  {
    /* circle through all elements and add all nodes */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_e; i++)
    {
      switch(e_enqire[set[setNr].elem[i]].type)
      {
        case 1:
          ipuf = 8;       /* HEXA8 */
        break;
        case 2:
          ipuf = 6;  /* PENTA6 */
        break;
        case 3:
          ipuf = 4;  /* TET4 */
        break;
        case 4:
          ipuf = 20; /* HEXA20 */
        break;
        case 5:
          ipuf = 15; /* PENTA15 */
        break;
        case 6:
          ipuf = 10; /* TET10 */
        break;
        case 7:
          ipuf = 3;  /* TRI3  */
        break;
        case 8:
          ipuf = 6;  /* TRI6  */
        break;
        case 9:
          ipuf = 4;  /* QUAD4 */
        break;
        case 10:
          ipuf = 8; /* QUAD8 */
        break;
        case 11:
          ipuf = 2; /* BEAM */
        break;
        case 12:
          ipuf = 3; /* BEAM3 */
        break;
      }
      for (j=0; j<ipuf; j++)
      {
	  //seta( setNr, "n", e_enqire[set[setNr].elem[i]].nod[j] );
          if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
            printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
          set[setNr].node[set[setNr].anz_n]= e_enqire[set[setNr].elem[i]].nod[j];
          set[setNr].anz_n++;
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
	//if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[n++]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
      //set[setNr].anz_n=n;
    }
  }
  return(1);
}



int completeSet_Lines( int setNr, int setNrbuf, int *elUsed, int flag )
{
  int i,j;


  if(!flag) /* up  */
  {
    if (set[setNr].anz_p>0)
    {
      /* suche abhaengige linien */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
	//    printf("pnt:%s\n", point[set[setNr].pnt[i]].name);
        for (j=0; j<anzGeo->l; j++)
        {
	  //printf("check line:%s %d  p:%d %d %d\n", line[j].name, elUsed[j], set[setNr].pnt[i], line[j].p1, line[j].p2);
          if( elUsed[j] ) continue;

          if( line[j].name != (char *)NULL )
          if(( set[setNr].pnt[i] == line[j].p1 )||( set[setNr].pnt[i] == line[j].p2 ))
          {
	    //printf("line:%s added\n", line[j].name);
            seta( setNrbuf, "l", j );
          }
        }
      }
    }
  }
  else /* down */
  {
    /* cyrcle through all lines and add all points, trks */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      j= set[setNr].line[i];
      seta( setNr, "p", line[j].p1 );
      seta( setNr, "p", line[j].p2 );
    }
  }
  return(1);
}



int completeSet_frame( int setNr, char type )
{
  int i,j, m, n, p, l, c, s;

    /* cyrcle through all sets and add all entities */
    for (i=0; i<set[setNr].anz_se; i++)
    {
      s= set[setNr].set[i];
      if((!type)||(type=='n'))
      for (j=0; j<set[s].anz_n; j++)
      {
        p=set[s].node[j];
        seta( setNr, "n", p );
      }
      if((!type)||(type=='f'))
      for (j=0; j<set[s].anz_f; j++)
      {
        p=set[s].face[j];
        seta( setNr, "f", p );
      }
      if((!type)||(type=='e'))
      for (j=0; j<set[s].anz_e; j++)
      {
        p=set[s].elem[j];
        seta( setNr, "e", p );
      }
      if((!type)||(type=='p'))
      for (j=0; j<set[s].anz_p; j++)
      {
        p=set[s].pnt[j];
        seta( setNr, "p", p );
      }
      if((!type)||(type=='l'))
      for (j=0; j<set[s].anz_l; j++)
      {
        p=set[s].line[j];
        seta( setNr, "l", p );
      }
      if((!type)||(type=='c'))
      for (j=0; j<set[s].anz_c; j++)
      {
        p=set[s].lcmb[j];
        seta( setNr, "c", p );
      }
      if((!type)||(type=='s'))
      for (j=0; j<set[s].anz_s; j++)
      {
        p=set[s].surf[j];
        seta( setNr, "s", p );
      }
      if((!type)||(type=='b'))
      for (j=0; j<set[s].anz_b; j++)
      {
        p=set[s].body[j];
        seta( setNr, "b", p );
      }
    }

    /* remove the appended sets */
    while(set[setNr].anz_se>0)
    {
      s= set[setNr].set[0];
      setr( setNr, "r", s);
    }


    /* cyrcle through all faces and add just the first 2 nodes */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_f; i++)
    {
        if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+2) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].node[set[setNr].anz_n]= face[set[setNr].face[i]].nod[0];
        set[setNr].anz_n++;
        set[setNr].node[set[setNr].anz_n]= face[set[setNr].face[i]].nod[1];
        set[setNr].anz_n++;
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
    }
    /* circle through all elements and add the first 2 nodes */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_e; i++)
    {
          if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+2) * sizeof(int))) == NULL )
            printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
          set[setNr].node[set[setNr].anz_n]= e_enqire[set[setNr].elem[i]].nod[0];
          set[setNr].anz_n++;
          set[setNr].node[set[setNr].anz_n]= e_enqire[set[setNr].elem[i]].nod[1];
          set[setNr].anz_n++;
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
    }

    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[setNr].anz_b; i++)
    {
      c= set[setNr].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( setNr, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs */
    for (i=0; i<set[setNr].anz_s; i++)
    {
      s= set[setNr].surf[i];
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( setNr, "l", l );
        }
        else
        {
          seta( setNr, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[setNr].anz_c; i++)
    {
      c= set[setNr].lcmb[i];
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( setNr, "l", l );
      }
    }
    /* cyrcle through all lines and add all points */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l= set[setNr].line[i];
      seta( setNr, "p", line[l].p1 );
      seta( setNr, "p", line[l].p2 );
      if (line[l].typ=='a') seta( setNr, "p", line[l].trk );
    }
  return(setNr);
}



int completeSet( char *setname, char *type )
{
  int i,j,k, m, n, p, l, c, s;
  int       setNr, ipuf;
  int       *nbody=NULL, *nsurf=NULL, *nline=NULL, *npoint=NULL ;

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in completeSet: set:%s does not exist\n", setname);
    return(-1);
  }

  if (toupper(type[0])=='U') /* up  */
  {
    if (set[setNr].anz_n>0)
    {
      /* suche abhaengige elemente */
      m=set[setNr].anz_e;
      for ( i=0; i<anz->e; i++)
      {
        ipuf=0;
        if (e_enqire[e_enqire[i].nr].type == 1) ipuf = 8;       /* HEXA8 */
        else if (e_enqire[e_enqire[i].nr].type == 2) ipuf = 6;  /* PENTA6 */
        else if (e_enqire[e_enqire[i].nr].type == 3) ipuf = 4;  /* TET4 */
        else if (e_enqire[e_enqire[i].nr].type == 4) ipuf = 20; /* HEXA20 */
        else if (e_enqire[e_enqire[i].nr].type == 5) ipuf = 15; /* PENTA15 */
        else if (e_enqire[e_enqire[i].nr].type == 6) ipuf = 10; /* TET10 */
        else if (e_enqire[e_enqire[i].nr].type == 7) ipuf = 3;  /* TRI3  */
        else if (e_enqire[e_enqire[i].nr].type == 8) ipuf = 6;  /* TRI6  */
        else if (e_enqire[e_enqire[i].nr].type == 9) ipuf = 4;  /* QUAD4 */
        else if (e_enqire[e_enqire[i].nr].type == 10) ipuf = 8; /* QUAD8 */
        else if (e_enqire[e_enqire[i].nr].type == 11) ipuf = 2; /* BEAM */
        else if (e_enqire[e_enqire[i].nr].type == 12) ipuf = 3; /* BEAM3 */
        if (ipuf!=0)
        {
          for (j=0; j<ipuf; j++)
          {
            if(ifind(&set[setNr].node, set[setNr].anz_n, e_enqire[e_enqire[i].nr].nod[j])>-1)
            {
	      // seta( setNr, "e", e_enqire[i].nr );
              if ( (set[setNr].elem = (int *)realloc((int *)set[setNr].elem, (set[setNr].anz_e+1) * sizeof(int))) == NULL )
                printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
              set[setNr].elem[set[setNr].anz_e]= e_enqire[i].nr;
              set[setNr].anz_e++;
              break;
            }
          }
        }
      }
      if(set[setNr].anz_e-m)
      {
        qsort( set[setNr].elem, set[setNr].anz_e, sizeof(int), (void *)compareInt );
        /* erase multiple entities */
        n=0;
        for(i=1; i<set[setNr].anz_e; i++)
        {
	  if(set[setNr].elem[n]!=set[setNr].elem[i]) set[setNr].elem[++n]=set[setNr].elem[i];
        }
        set[setNr].anz_e=n+1;
      }
    }
    if (set[setNr].anz_f>0)
    {
      /* suche abhaengige elemente */
      m=set[setNr].anz_e;
      for ( i=0; i<set[setNr].anz_f; i++)
      {
	// seta( setNr, "e", face[set[setNr].face[i]].elem_nr );
        if ( (set[setNr].elem = (int *)realloc((int *)set[setNr].elem, (set[setNr].anz_e+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].elem[set[setNr].anz_e]= face[set[setNr].face[i]].elem_nr;
        set[setNr].anz_e++;
      }
      if(set[setNr].anz_e-m)
      {
        qsort( set[setNr].elem, set[setNr].anz_e, sizeof(int), (void *)compareInt );
        /* erase multiple entities */
        n=0;
        for(i=1; i<set[setNr].anz_e; i++)
        {
	  if(set[setNr].elem[n]!=set[setNr].elem[i]) set[setNr].elem[++n]=set[setNr].elem[i];
        }
        set[setNr].anz_e=n+1;
      }
    }
    if (set[setNr].anz_p>0)
    {
      /* suche abhaengige linien */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
        for (j=0; j<anzGeo->l; j++)
        {
          if( line[j].name != (char *)NULL )
          if(( set[setNr].pnt[i] == line[j].p1 )||( set[setNr].pnt[i] == line[j].p2 ))
          {
            seta( setNr, "l", j );
          }
        }
      }
      /* suche abhaengige shapes */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
        for (j=0; j<anzGeo->sh; j++)
        {
          if(( shape[j].name != (char *)NULL )&&( shape[j].type!=4))
          {
            /* 0:plane, 1:cyl, 2: cone, 3:sph, 4:nurbs, 5:tor */
            /* 0:3p, 1:3p, 2:4p 3:7p 4:p[0]=nurbsindx 5:4p*/
            
            if(shape[j].type<2) l=3;
            else if(shape[j].type==2) l=4;
            else if(shape[j].type==3) l=7;
            else if(shape[j].type==5) l=4;
            else continue;
            for(k=0; k<l; k++)
	    { 
              if ( set[setNr].pnt[i] == shape[j].p[k] )
              {
                seta( setNr, "sh", j);
              }
	    }
          }
        }
      }
      /* suche abhaengige nurl */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
        for (j=0; j<anzGeo->nurl; j++)
        {
          if( nurbl[j].name != (char *)NULL )
          {
            for(k=0; k<nurbl[j].u_npnt; k++)
	    { 
              if ( set[setNr].pnt[i] == nurbl[j].ctlpnt[k] )
              {
                seta( setNr, "L", j);
              }
	    }
          }
        }
      }
      /* suche abhaengige nurs */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
        for (j=0; j<anzGeo->nurs; j++)
        {
          if( nurbs[j].name != (char *)NULL )
          {
            for(k=0; k<nurbs[j].u_npnt; k++)
	    { 
              for(l=0; l<nurbs[j].v_npnt; l++)
	      { 
                if ( set[setNr].pnt[i] == nurbs[j].ctlpnt[k][l] )
                {
                  seta( setNr, "S", j);
		}
              }
	    }
          }
        }
      }
    }
    if (set[setNr].anz_l>0)
    {
      /* suche abhaengige lcmbs */
      for ( i=0; i<set[setNr].anz_l; i++)
      {
        for (j=0; j<anzGeo->c; j++)
        {
          if( lcmb[j].name != (char *)NULL )
          for (n=0; n<lcmb[j].nl; n++)
          {
            if( set[setNr].line[i] == lcmb[j].l[n] )
            {
              seta( setNr, "c", j );
            }
          }
        }
      }
      /* suche abhaengige surfs */
      for ( i=0; i<set[setNr].anz_l; i++)
      {
        for (j=0; j<anzGeo->s; j++)
        {
          if( surf[j].name != (char *)NULL )
          for (n=0; n<surf[j].nl; n++)
          {
            if(( set[setNr].line[i] == surf[j].l[n] )&&( surf[j].typ[n] == 'l' ))
            {
              seta( setNr, "s", j);
            }
          }
        }
      }
    }
    if (set[setNr].anz_c>0)
    {
      /* suche abhaengige surfaces */
      for ( i=0; i<set[setNr].anz_c; i++)
      {
        for (j=0; j<anzGeo->s; j++)
        {
          if( surf[j].name != (char *)NULL )
          for (n=0; n<surf[j].nl; n++)
          {
            if(( set[setNr].lcmb[i] == surf[j].l[n] )&&( surf[j].typ[n] == 'c' ))
            {
              seta( setNr, "s", j );
            }
          }
        }
      }
    }
    if (set[setNr].anz_s>0)
    {
      /* suche abhaengige bodys */
      for ( i=0; i<set[setNr].anz_s; i++)
      {
        for (j=0; j<anzGeo->b; j++)
        {
          if( body[j].name != (char *)NULL )
          for (n=0; n<body[j].ns; n++)
          {
            if ( set[setNr].surf[i] == body[j].s[n] )
            {
              seta( setNr, "b", j);
            }
          }
        }
      }
    }
    if (set[setNr].anz_nurs>0)
    {
      /* suche abhaengige shapes */
      for ( i=0; i<set[setNr].anz_nurs; i++)
      {
        for (j=0; j<anzGeo->sh; j++)
        {
          if(( shape[j].name != (char *)NULL )||( shape[j].type==4))
          {
            if ( set[setNr].nurs[i] == shape[j].p[0] )
            {
              seta( setNr, "sh", j);
            }
          }
        }
      }
    }
    if (set[setNr].anz_sh>0)
    {
      /* suche abhaengige surfaces */
      for ( i=0; i<set[setNr].anz_sh; i++)
      {
        for (j=0; j<anzGeo->s; j++)
        {
          if( surf[j].name != (char *)NULL )
          {
            if ( set[setNr].shp[i] == surf[j].sh )
            {
              seta( setNr, "s", j);
            }
          }
        }
      }
    }
  }

  else if (toupper(type[0])=='D') /* down */
  {
    /* get the faces */
    completeSet( set[setNr].name, "f" );

    /* cyrcle through all nodes and add all geometric entities */
    if(set[setNr].anz_n)
    {
      /* mark the related geometry */
      if( (nbody=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      if( (nsurf=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      if( (nline=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      if( (npoint=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      for (i=0; i<=anz->nmax; i++)
      {
        nbody[i]=nsurf[i]=nline[i]=npoint[i]=-1;
      }

      for (i=0; i<anzGeo->b; i++)
      {
        for (j=0; j<body[i].nn; j++){ nbody[body[i].nod[j]]=i; }
      }
      for (i=0; i<anzGeo->s; i++)
      {
        for (j=0; j<surf[i].nn; j++){ nsurf[surf[i].nod[j]]=i; }
      }
      for (i=0; i<anzGeo->l; i++)
      {
        for (j=0; j<line[i].nn; j++){ nline[line[i].nod[j]]=i; }
      }
      for (i=0; i<anzGeo->p; i++)
      {
        for (j=0; j<point[i].nn; j++){ npoint[point[i].nod[j]]=i; }
      }
      for (i=0; i<set[setNr].anz_n; i++)
      {
        /* check if the node is relevant */
        if(nbody[set[setNr].node[i]]>-1) seta( setNr, "b", nbody[set[setNr].node[i]]);
        if(nsurf[set[setNr].node[i]]>-1) seta( setNr, "s", nsurf[set[setNr].node[i]]);
        if(nline[set[setNr].node[i]]>-1) seta( setNr, "l", nline[set[setNr].node[i]]);
        if(npoint[set[setNr].node[i]]>-1) seta( setNr, "p", npoint[set[setNr].node[i]]);
      }

      mfail_comp:;
      free(nbody);
      free(nsurf);
      free(nline);
      free(npoint);
    }

    /* cyrcle through all faces and add all nodes */
    /* nodes have to be completed before because otherwhise all geometry with their nodes would be included */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_f; i++)
    {
      if (face[set[setNr].face[i]].type == 7) n = 3;  /* TRI3  */
      else if (face[set[setNr].face[i]].type == 8) n = 6;  /* TRI6  */
      else if (face[set[setNr].face[i]].type == 9) n = 4;  /* QUAD4 */
      else if (face[set[setNr].face[i]].type == 10) n = 8; /* QUAD8 */
      else if (face[set[setNr].face[i]].type == 11) n = 2; /* BEAM2 */
      else if (face[set[setNr].face[i]].type == 12) n = 3; /* BEAM3 */
      else n=0;
      for (j=0; j<n; j++)
      {
        // seta( setNr, "n", face[set[setNr].face[i]].nod[j] );
        if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].node[set[setNr].anz_n]= face[set[setNr].face[i]].nod[j];
        set[setNr].anz_n++;
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
    }
    /* circle through all elements and add all nodes and faces */
    /* nodes have to be completed before because otherwhise all geometry with their nodes would be included */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_e; i++)
    {
      if (e_enqire[set[setNr].elem[i]].type == 1) n = 8;       /* HEXA8 */
      else if (e_enqire[set[setNr].elem[i]].type == 2) n = 6;  /* PENTA6 */
      else if (e_enqire[set[setNr].elem[i]].type == 3) n = 4;  /* TET4 */
      else if (e_enqire[set[setNr].elem[i]].type == 4) n = 20; /* HEXA20 */
      else if (e_enqire[set[setNr].elem[i]].type == 5) n = 15; /* PENTA15 */
      else if (e_enqire[set[setNr].elem[i]].type == 6) n = 10; /* TET10 */
      else if (e_enqire[set[setNr].elem[i]].type == 7) n = 3;  /* TRI3  */
      else if (e_enqire[set[setNr].elem[i]].type == 8) n = 6;  /* TRI6  */
      else if (e_enqire[set[setNr].elem[i]].type == 9) n = 4;  /* QUAD4 */
      else if (e_enqire[set[setNr].elem[i]].type == 10) n = 8; /* QUAD8 */
      else if (e_enqire[set[setNr].elem[i]].type == 11) n = 2; /* BEAM2 */
      else if (e_enqire[set[setNr].elem[i]].type == 12) n = 3; /* BEAM3 */
      else n=0;
      for (j=0; j<n; j++)
      {
  	  // seta( setNr, "n", e_enqire[set[setNr].elem[i]].nod[j] );
          if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
            printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
          set[setNr].node[set[setNr].anz_n]= e_enqire[set[setNr].elem[i]].nod[j];
          set[setNr].anz_n++;
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
    }

    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[setNr].anz_b; i++)
    {
      c= set[setNr].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( setNr, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs and nurbs */
    for (i=0; i<set[setNr].anz_s; i++)
    {
      s= set[setNr].surf[i];
      if(surf[s].sh>-1) seta( setNr, "sh", surf[s].sh );
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( setNr, "l", l );
        }
        else
        {
          seta( setNr, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[setNr].anz_c; i++)
    {
      c= set[setNr].lcmb[i];
      if( lcmb[c].name != (char *)NULL) for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( setNr, "l", l );
      }
    }
    /* cyrcle through all lines and add all points, trks */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l= set[setNr].line[i];
      seta( setNr, "p", line[l].p1 );
      seta( setNr, "p", line[l].p2 );
      if (line[l].typ=='a') seta( setNr, "p", line[l].trk );
      if (line[l].typ=='s') seta( setNr, "se", line[l].trk );
    }
    /* cyrcle through all shapes and add all points and nurbs */
    for (i=0; i<set[setNr].anz_sh; i++)
    {
      s= set[setNr].shp[i];
      if(shape[s].type==0) for (j=0; j<3; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==1) for (j=0; j<3; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==2) for (j=0; j<4; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==3) for (j=0; j<7; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==4) seta( setNr, "S", shape[s].p[0] );
      else if(shape[s].type==5) for (j=0; j<4; j++) seta( setNr, "p", shape[s].p[j] );
    }
    /* cyrcle through all nurbs and add all points */
    for (i=0; i<set[setNr].anz_nurs; i++)
    {
      s= set[setNr].nurs[i];
      for (j=0; j<nurbs[s].u_npnt; j++)
        for (k=0; k<nurbs[s].v_npnt; k++)
          seta( setNr, "p", nurbs[s].ctlpnt[j][k] );
    }
    /* cyrcle through all sets and add all entities */
    for (i=0; i<set[setNr].anz_se; i++)
    {
      s= set[setNr].set[i];
      for (j=0; j<set[s].anz_n; j++)
      {
        p=set[s].node[j];
        seta( setNr, "n", p );
      }
      for (j=0; j<set[s].anz_f; j++)
      {
        p=set[s].face[j];
        seta( setNr, "f", p );
      }
      for (j=0; j<set[s].anz_e; j++)
      {
        p=set[s].elem[j];
        seta( setNr, "e", p );
      }
      for (j=0; j<set[s].anz_p; j++)
      {
        p=set[s].pnt[j];
        seta( setNr, "p", p );
      }
      for (j=0; j<set[s].anz_l; j++)
      {
        p=set[s].line[j];
        seta( setNr, "l", p );
      }
      for (j=0; j<set[s].anz_c; j++)
      {
        p=set[s].lcmb[j];
        seta( setNr, "c", p );
      }
      for (j=0; j<set[s].anz_s; j++)
      {
        p=set[s].surf[j];
        seta( setNr, "s", p );
      }
      for (j=0; j<set[s].anz_b; j++)
      {
        p=set[s].body[j];
        seta( setNr, "b", p );
      }
      for (j=0; j<set[s].anz_sh; j++)
      {
        p=set[s].shp[j];
        seta( setNr, "sh", p );
      }
    }

    /* remove the appended sets */
    while(set[setNr].anz_se>0)
    {
      s= set[setNr].set[0];
      setr( setNr, "r", s);
    }

  }

  else if (toupper(type[0])=='F') /* faces */
  {
    if ( setNr==getSetNr(specialset->mesh)||( set[setNr].type==1)||(setNr==getSetNr(specialset->nomesh))) return(-1);
    
    /* add the faces to the sets which fit fully to the included nodes */
    if(printFlag) printf(" include faces in sets\n");

    /* go over all faces and look if their nodes are included in the set setNr */
    m=set[setNr].anz_f;
    for (i=0; i<anz->f; i++)
    { 
      if (face[i].type == 7) n = 3;  /* TRI3  */
      else if (face[i].type == 8) n = 6;  /* TRI6  */
      else if (face[i].type == 9) n = 4;  /* QUAD4 */
      else if (face[i].type == 10) n = 8; /* QUAD8 */
      else if (face[i].type == 11) n = 2; /* beam2 */
      else if (face[i].type == 12) n = 3; /* beam3 */
      else n=0;
      k=0;
      for (j=0; j<n; j++)
      {
	if(ifind(&set[setNr].node, set[setNr].anz_n, face[i].nod[j])>-1) k++;
      }
      if(k==n)
      {
	// seta( setNr, "f", i );
        if ( (set[setNr].face = (int *)realloc((int *)set[setNr].face, (set[setNr].anz_f+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].face[set[setNr].anz_f]= i;
        set[setNr].anz_f++;
      }
    }
    if(set[setNr].anz_f-m)
    {
      qsort( set[setNr].face, set[setNr].anz_f, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_f; i++)
      {
	if(set[setNr].face[n]!=set[setNr].face[i]) set[setNr].face[++n]=set[setNr].face[i];
      }
      set[setNr].anz_f=n+1;
    }

    /* go over all faces and look if their elements are included in the set setNr */
    m=set[setNr].anz_f;
    for (i=0; i<anz->f; i++)
    { 
      if(ifind(&set[setNr].elem, set[setNr].anz_e, face[i].elem_nr)>-1)
      {
	// seta( setNr, "f", i );
        if ( (set[setNr].face = (int *)realloc((int *)set[setNr].face, (set[setNr].anz_f+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].face[set[setNr].anz_f]= i;
        set[setNr].anz_f++;
      }
    }
    if(set[setNr].anz_f-m)
    {
      qsort( set[setNr].face, set[setNr].anz_f, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_f; i++)
      {
	if(set[setNr].face[n]!=set[setNr].face[i]) set[setNr].face[++n]=set[setNr].face[i];
      }
      set[setNr].anz_f=n+1;
    }
  }

  else if (toupper(type[0])=='C') /* LCMB's */
  {
    /* cyrcle through all lcmbs and add it if all lines are part of the set */
    for (c=0; c<anzGeo->c; c++) if( lcmb[c].name != (char *)NULL )
    {
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        if(0>ifind(&set[setNr].line, set[setNr].anz_l, l )) break;
      }
      if(j==lcmb[c].nl) seta( setNr, "c", c );
    }
  }

  else if (toupper(type[0])=='E') /* edges */
  {
    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[setNr].anz_b; i++)
    {
      c= set[setNr].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( setNr, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs  */
    for (i=0; i<set[setNr].anz_s; i++)
    {
      s= set[setNr].surf[i];
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( setNr, "l", l );
        }
        else
        {
          seta( setNr, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[setNr].anz_c; i++)
    {
      c= set[setNr].lcmb[i];
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( setNr, "l", l );
      }
    }
    /* cyrcle through all lines and add all corner-points */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l= set[setNr].line[i];
      seta( setNr, "p", line[l].p1 );
      seta( setNr, "p", line[l].p2 );
    }
  }
  else printf(" Command not recognized, use up down or edges\n");
  return(setNr);
}



/* send the interiour of the surfaces in stl format (not used ) */

int sendTriangles(  char *setname )
{
  int i,j,k,n,s, setNr, tris=0;
  FILE *handle;

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in completeSet: set:%s does not exist\n", setname);
    return(-1);
  }

  sprintf(buffer, "%s.stl", setname);
  handle = fopen (buffer, "w");
  if (handle==NULL)
  {
    printf (" ERROR in sendTriangles: The input file \"%s\" could not be opened.\n\n", datin);
    return(-1);
  }
  else  printf ("\n%s opened\n",buffer);

  /* write the interiour of the surface */
  fprintf( handle, "solid\n");
  for (i=0; i<set[setNr].anz_s; i++)
  {
    s= set[setNr].surf[i];

    n=0;
    while((surf[s].npgn-n))
    {
      n++; /* jump over the polygon token (ie.GL_POLYGON_TOKEN) */
      j=surf[s].pgn[n++];
      fprintf( handle, "  facet normal %e %e %e\n",surf[s].pgn[n],surf[s].pgn[n+1],surf[s].pgn[n+2]); n+=3;
      fprintf( handle, "    outer loop\n");
      for(k=0; k<j; k++)
      {
        fprintf( handle, "      vertex %e %e %e\n",
          surf[s].pgn[n]*scale->w+scale->x,
          surf[s].pgn[n+1]*scale->w+scale->y,
          surf[s].pgn[n+2]*scale->w+scale->z); n+=3;
      }
      tris++;
      fprintf( handle, "    endloop\n");
      fprintf( handle, "  endfacet\n");
    }
  }
  fprintf( handle, "endsolid\n");
  printf("wrote %d triangles\n", tris);
  fclose(handle);
  return(1);
}



void pre_write( char *record )
{
  char  setname[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH], **dat;
  int          length, anz_l, setNr, i,j,k,l=0,s,v,n,b;
  char typ, buffer[MAX_LINE_LENGTH];
  static char  **val=NULL;
  double        f[3]={0.,0.,0.};
  int bset;
  int bouNr;
  NodeBlockbou *blockbou=NULL;
  double v1[3], v2[3], v3[3];
  int tr6[4][3]={{0,3,5},{4,5,3},{3,1,4},{4,2,5}};
  int qu4[2][3]={{0,1,2},{2,3,0}};
  int qu8[8][3]={{0,4,8},{8,4,1},{1,5,8},{8,5,2},{2,6,8},{8,6,3},{3,7,8},{8,7,0}};

  FILE *handle=NULL;

  if(val){ for(i=0; i<5; i++) free(val[i]);  free(val); }
  if(( val=(char **)malloc( 5*sizeof(char *)) )==NULL)
  { printf(" ERROR: malloc failure\n\n" ); return; }
  for(i=0; i<5; i++)
  {
    if(( val[i]=(char *)malloc( MAX_LINE_LENGTH*sizeof(char)) )==NULL)
    { printf(" ERROR: malloc failure\n\n" ); return; }
    val[i][0]=0;
  }

  /* clean the buffers */
  for(i=0; i<MAX_LINE_LENGTH; i++) buffer[i]=setname[i]=format[i]=type[i]='\0';
  for(i=0; i<MAX_LINE_LENGTH; i++) for(j=0; j<5; j++) val[j][i]='\0';

  length = sscanf(record, "%s%s%s%s%s%s%s%s", setname, format, type, val[0], val[1], val[2], val[3], val[4]);
  if (length<2)
  {
    if (compare( setname, "init", 4)== 4)
    {
      sprintf(buffer,"init.fbl");
      handle = fopen (buffer, "w+b");
      if (handle==NULL) { printf ("\nThe output file \"%s\" could not be opened.\n\n", buffer ); return; }
      writeInit(handle);
      fclose(handle);
      printf (" ready\n");
      return;
    }
    else { printf(" ERROR: Format missing!\n"); return; }
  }

  /* replace 'lc' by 'ds' */
  if(compare( type, "lc", 2)== 2) { type[0]='d'; type[1]='s'; }
  for(i=0; i<4; i++) if(compare( val[i], "lc", 2)== 2) { val[i][0]='d'; val[i][1]='s'; }

  operateAlias( setname, "se" );
  printf (" please wait for 'ready'\n");

  if (compare( format, "bp", 2)== 2)
  {
    if(getSetNr(specialset->zap)>-1)
    {
      errMsg("ERROR: special set:%s is not empty. Please delete the set before\n", specialset->zap);
      return;
    }
    if (length>2)
    {
      if (compare( type, "c", 1)== 1)
      {
      completeSet( setname, "do" );
      }
    }
    descalPoints( anzGeo->p, point, scale);
    writebp( setname, anz, anzGeo, point, set );
    scalPoints( anzGeo->p, point, scale);
  }
  else if (compare( format, "fbd", 3)== 3)
  {
    if(getSetNr(specialset->zap)>-1)
    {
      errMsg("ERROR: special set:%s is not empty. Please delete the set before\n", specialset->zap);
      return;
    }
    if (length>2)
    {
      if (compare( type, "c", 1)== 1)
      {
        completeSet( setname, "do" );
      }
      if (compare( type, "f", 1)== 1)
      {
        // write point coordinates in long float, else in exp
        strcpy( format, "lf" );
      }
      if (compare( type, "e", 1)== 1)
      {
        sprintf(buffer,"%s_edges.fbd", setname);
        handle = fopen (buffer, "w+b");
        if (handle==NULL) { printf ("\nThe output file \"%s\" could not be opened.\n\n", buffer ); return; }
        descalNodes ( anz->n, node, scale);
        for (i=0; i<anz->g; i++ )
        {
          fprintf(handle, " pnt a%d %f %f %f\n", i, node[edge[i].p1].nx, node[edge[i].p1].ny, node[edge[i].p1].nz );
          fprintf(handle, " pnt b%d %f %f %f\n", i, node[edge[i].p2].nx, node[edge[i].p2].ny, node[edge[i].p2].nz );
          fprintf(handle, " line l%d a%d b%d 1\n",i,i,i);
        }
        fprintf(handle, " merg p all 0.00001\n");
        scalNodes ( anz->n, node, scale );
        fclose(handle);
        printf (" ready\n");
        return;
      }
    }
    descalPoints( anzGeo->p, point, scale);
    writefbd( setname, format );
    scalPoints( anzGeo->p, point, scale);
  }
  else if (compare( format, "ng", 2)== 2)
  {
    if(compareStrings( setname, "all" )<=0)
    {
	printf("ERROR: netgen-format was requested for a subset:%s. Up to now only the faces of the full mesh can be written (set all)\n",setname);
      return;
    }
    if(anz->f<1)
    {
      printf("ERROR: no faces exist\n");
      return;
    }

    descalNodes ( anz->n, node, scale);

    handle = fopen ("mesh.ng", "w+b");
    if (handle==NULL) { printf ("\nThe output file \"mesh.ng\" could not be opened.\n\n"); return; }
    printf (" write surface-mesh data for the netgen-tetmesher ng_vol \n");

    fprintf (handle, "%d\n", anz->n);
    for (i=0; i<anz->n; i++)
    {
      fprintf (handle, "%.12e %.12e %.12e\n", node[node[i].nr].nx, node[node[i].nr].ny, node[node[i].nr].nz);
    }
    i=0;
    for (j=0; j<anz->f; j++)
    {
      if ((face[j].type == 7)|| (face[j].type == 8)) i++;
      else if ((face[j].type == 9)|| (face[j].type == 10)) i+=2;
    }
    fprintf (handle, "%d\n", i);
/*
    fprintf (handle, "%d\n", anz->e);
    for (i=0; i<anz->e; i++)
    {
      j=e_enqire[i].nr;
      if (e_enqire[j].type == 7)
      {
        fprintf (handle, "%d %d %d\n",
         node[e_enqire[j].nod[0]].indx+1, node[e_enqire[j].nod[1]].indx+1, node[e_enqire[j].nod[2]].indx+1);
      }
      else
      {
        printf (" WARNING: elem(%d) not a known type (%d)\n", e_enqire[j].nr, e_enqire[j].type);
      }
    }
*/
    for (j=0; j<anz->f; j++)
    {
      if ((face[j].type == 7)|| (face[j].type == 8))
      {
        fprintf (handle, "%d %d %d\n",
         node[face[j].nod[0]].indx+1, node[face[j].nod[1]].indx+1, node[face[j].nod[2]].indx+1);
      }
      else if ((face[j].type == 9)|| (face[j].type == 10))
      {
        fprintf (handle, "%d %d %d\n",
         node[face[j].nod[0]].indx+1, node[face[j].nod[1]].indx+1, node[face[j].nod[2]].indx+1);
        fprintf (handle, "%d %d %d\n",
         node[face[j].nod[2]].indx+1, node[face[j].nod[3]].indx+1, node[face[j].nod[0]].indx+1);
      }
      else
      {
        printf (" WARNING: face(%d) not a known type (%d)\n", j, face[j].type);
      }
    }
    fclose(handle);

    scalNodes ( anz->n, node, scale);
  }
  else if (compare( format, "frd", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "nor", 3)== 3)
        sendSurfNormalen( setname, format, anz, node, e_enqire, lcase, scale );
      else if (compare( type, "sur", 3)== 3)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[1],val[0]);
        strcpy(val[0],type);
	//val[1]=val[0];
        //val[0]=type;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, val);
        anz->l=anz_l;
      }
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else if ((compare( type, "ds", 2)== 2)||(compare( type, "bin", 2)== 2)||(compare( type, "dbin", 3)== 3))
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, val);
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "darwin", 3)== 3)
  {
    anz_l=anz->l;
    anz->l=anz->olc;
    strcpy(val[1], val[0]);
    strcpy(val[0], type);
    senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
    anz->l=anz_l;
  }
  /* list of entities */
  else if (compare( format, "lst", 3)== 3)
  {
    senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
  }
  else if (compare( format, "seq", 3)== 3)
  {
    senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
  }
  /* gagemap */
  else if (compare( format, "gmp", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    return;
  }
  /* patran neutral file */
  else if (compare( format, "pat", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
      {
        /* if setname == "all" -> send all sets */
        if(compareStrings( setname, "all" )>0)
        {
          for(i=0; i<anz->sets; i++)
	  {
            if((set[i].name!=NULL)&&(set[i].name[0]!='+')&&(set[i].name[0]!='-')&&(compareStrings(set[i].name, "all" )<=0)&&((set[i].anz_n>0)||(set[i].anz_e>0))) sendNames( set[i].name, format, anz, node, e_enqire );
	  }
	} 
        else sendNames( setname, format, anz, node, e_enqire );
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    return;
  }
  else if (compare( format, "skv", 3)== 3)
  {
    sendSKV( setname, format, anz, node, e_enqire, type, val[0], val[1] );
  }
  else if (compare( format, "abq", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "quadlin", 3)== 3)
        sendQuadLin( setname, format, anz, node, e_enqire );
      else if (compare( type, "trac", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], "track" );
      else if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "film", 3)== 3)
        sendFilm( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3], val[4] );
      else if (compare( type, "radiate", 3)== 3)
        sendRadiate( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3], val[4] );
      else if (compare( type, "mflow", 5)== 5)
        sendDflux( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 1 );
      else if (compare( type, "dflux", 5)== 5)
        sendDflux( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "force", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spcf", 4)== 4)
        sendSPCF( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "cflux", 3)== 3)
        sendCflux( setname, format, anz, node, e_enqire, "t", val[0] );
      else if (compare( type, "slidef", 6)== 6)
        sendSlidersf( setname, format, val[0]  );
      else if (compare( type, "slide", 3)== 3)
        sendSliders( setname, format, anz, node, e_enqire, val[0]  );
      else if (compare( type, "names", 3)== 3)
      {
        /* if setname == "all" -> send all sets */
        if(compareStrings( setname, "all" )>0)
        {
          for(i=0; i<anz->sets; i++)
	  {
            if((set[i].name!=NULL)&&(set[i].name[0]!='+')&&(set[i].name[0]!='-')&&(compareStrings(set[i].name, "all" )<=0)&&((set[i].anz_n>0)||(set[i].anz_e>0))) sendNames( set[i].name, format, anz, node, e_enqire );
	  }
	} 
        sendNames( setname, format, anz, node, e_enqire );
      }
      else if (compare( type, "raw", 3)== 3)
      {
        sendNames( setname, type, anz, node, e_enqire );
        sendSurfaces( setname, type, anz, node, e_enqire, val[0] );
      }
      else if (compare( type, "sur", 3)== 3)
      {
        /* if setname == "all" -> send all sets */
        if(compareStrings( setname, "all" )>0)
        {
          for(i=0; i<anz->sets; i++)
	  {
            if((set[i].name!=NULL)&&(set[i].name[0]!='+')&&(set[i].name[0]!='-')&&(compareStrings(set[i].name, "all" )<=0)&&((set[i].anz_f>0))) { sendSurfaces( set[i].name, format, anz, node, e_enqire, val[0] ); }
	  }
	} 
        else sendSurfaces( setname, format, anz, node, e_enqire, val[0] );
      }
      else if (compare( type, "mpc", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendMpc( setname, format, val[0], f ); }
      else if (compare( type, "ds", 2)== 2)
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "tmf", 2)== 2)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "crp", 2)== 2)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[3], val[2]); /* write-frequency */
        strcpy(val[2], val[1]); /* reference-speed */
        strcpy(val[1], val[0]); /* time-factor */
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "sta", 2)== 2)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[2], val[0]); /* reference-speed */
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "c", 1)== strlen(type))
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  /* changes from Paul CARRICO */
  else if (compare( format, "aster", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  /* changes from Paul CARRICO */
  else if (compare( format, "sam", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "adh", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "ans", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "for", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "slide", 3)== 3)
        sendSliders( setname, format, anz, node, e_enqire, val[0]  );
      else if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else if ((compare( type, "ds", 2)== 2)||(compare( type, "tmf", 2)== 2))
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "nas", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "for", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else if (compare( type, "mpc", 3)== 3)
      { 
        length = sscanf(record, "%*s%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendMpc( setname, format, val[0], f ); 
      }
      else if (compare( type, "ds", 2)== 2)
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "tcg", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "for", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "dolfyn", 3)== 3)
  {
    length= strsplt( record, ' ', &dat);
    descalNodes ( anz->n, node, scale);
    if(length>2) write2dolfyn( setname, length-2, &dat[2], anz, node, face, e_enqire, set );
    scalNodes ( anz->n, node, scale);

    /* free dat */
    for(i=0; i<length; i++) free(dat[i]); free(dat);
  }
  else if (compare( format, "duns", 3)== 3)
  {
    if (compare( type, "names", 3)== 3) sendNames( setname, format, anz, node, e_enqire );
    else
    {
      length= strsplt( record, ' ', &dat);

      /* initialize the bctype with "0" */
      for( b=0; b<anz->b; b++)
      {
        if(nBlock[b].dim==2)
        {
          s=nBlock[b].geo;
          for(j=0; j<surf[s].nl; j++) nBlock[b].bctype[j][0]=0;
        }
        if(nBlock[b].dim==3)
        {
          s=nBlock[b].geo;
          for(j=0; j<body[s].ns; j++) nBlock[b].bctype[j][0]=0;
        }
      }

      /* add the grid-blocks to the type of boundary */
      bouNr=-1;
      n=2; while(n<length)
      {
        /* search the corresponding blocks */
        bset=getSetNr(dat[n+1]);
        if(bset<0)
        {
          printf("boundary set:%s does not exist\n", dat[n+1]);
          return;
        }
        bouNr++;
        if((blockbou=(NodeBlockbou *)realloc((NodeBlockbou *)blockbou, (bouNr+1)*sizeof(NodeBlockbou)) )==NULL)
        { printf(" ERROR: realloc failure, blockbou:%d could not be allocated\n\n", bouNr ); return; }
        strcpy(blockbou[bouNr].name,set[bset].name);
        blockbou[bouNr].surf=NULL;
        blockbou[bouNr].nBlock=NULL;
        blockbou[bouNr].side=NULL;
        blockbou[bouNr].surfs=0;

        /* add the lcmb to the set */
        for(i=0; i<set[bset].anz_l; i++)
        {
          for(j=0; j<anzGeo->c; j++) if( lcmb[j].name != (char *)NULL )
          {
            for(l=0; l<lcmb[j].nl; l++)
            {
              if(lcmb[j].l[l]==set[bset].line[i]) seta(bset, "c", j);
            }
          }
        }

        /* look if a member of the set is used by a block */
        for( b=0; b<anz->b; b++)
        {
          if(nBlock[b].dim==2)
          {
            s=nBlock[b].geo;
            for(j=0; j<surf[s].nl; j++)
            {
              if(nBlock[b].map[j][0]==-1)   // has no neighbor
              {
                if((blockbou[bouNr].surf=(int *)realloc((int *)blockbou[bouNr].surf, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
                if((blockbou[bouNr].nBlock=(int *)realloc((int *)blockbou[bouNr].nBlock, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
                if((blockbou[bouNr].side=(int *)realloc((int *)blockbou[bouNr].side, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }

                l=surf[s].l[nBlock[b].bcface[j]];
                typ=surf[s].typ[nBlock[b].bcface[j]];
		
                if(typ=='l')
                {
                  for(i=0; i<set[bset].anz_l; i++)
                  {
                    if(l==set[bset].line[i])
                    {
                      printf("set:%s btype:%s bouNr:%d line[%d]:%s block:%d duns-side[%d]:%d\n",set[bset].name, dat[n],bouNr,blockbou[bouNr].surfs, line[set[bset].line[i]].name, b, j, nBlock[b].neighbor[j]);
                      blockbou[bouNr].surf[blockbou[bouNr].surfs] = nBlock[b].neighbor[j];
                      strcpy(blockbou[bouNr].bctype, dat[n]);
                      blockbou[bouNr].nBlock[blockbou[bouNr].surfs] = b;
                      blockbou[bouNr].side[blockbou[bouNr].surfs] = j;
                      blockbou[bouNr].surfs++;
                      break;
                    }
                  }
                }
                if(typ=='c')
                {
                  for(i=0; i<set[bset].anz_c; i++)
                  {
                    if(l==set[bset].lcmb[i])
                    {
                      printf("set:%s btype:%s bouNr:%d lcmb[%d]:%s block:%d duns-side:%d\n",set[bset].name, dat[n],bouNr,blockbou[bouNr].surfs, lcmb[set[bset].lcmb[i]].name, b, nBlock[b].neighbor[j]);
                      blockbou[bouNr].surf[blockbou[bouNr].surfs] = nBlock[b].neighbor[j];
                      strcpy(blockbou[bouNr].bctype, dat[n]);
                      blockbou[bouNr].nBlock[blockbou[bouNr].surfs] = b;
                      blockbou[bouNr].side[blockbou[bouNr].surfs] = j;
                      blockbou[bouNr].surfs++;
                      break;
                    }
                  }
                }
	      }
            }
          }
          if(nBlock[b].dim==3)
	  {
            v=nBlock[b].geo;
            for(i=0; i<set[bset].anz_s; i++)
            {
              for(j=0; j<body[v].ns; j++)
              {
                if(nBlock[b].map[j][0]==-1)   // has no neighbor
		{
		  s=body[v].s[nBlock[b].bcface[j]];
                  if(s==set[bset].surf[i])
                  {
                    if((blockbou[bouNr].surf=(int *)realloc((int *)blockbou[bouNr].surf, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                    { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
                    if((blockbou[bouNr].nBlock=(int *)realloc((int *)blockbou[bouNr].nBlock, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                    { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
                    if((blockbou[bouNr].side=(int *)realloc((int *)blockbou[bouNr].side, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                    { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }

		    printf("body:%s bsurf:%s surf:%s\n", body[v].name, surf[body[v].s[nBlock[b].bcface[j]]].name, surf[set[bset].surf[i]].name);
                    printf("set:%s btype:%s bouNr:%d surf[%d]:%s block:%d duns-side[%d]:%d\n",set[bset].name, dat[n],bouNr,blockbou[bouNr].surfs, surf[set[bset].surf[i]].name, b, j, nBlock[b].neighbor[j]);
                    blockbou[bouNr].surf[blockbou[bouNr].surfs] = nBlock[b].neighbor[j];
                    strcpy(blockbou[bouNr].bctype, dat[n]);
                    blockbou[bouNr].nBlock[blockbou[bouNr].surfs] = b;
                    blockbou[bouNr].side[blockbou[bouNr].surfs] = j;
                    blockbou[bouNr].surfs++;
                    break;
                  }
                }
              }
            }
	  }
        }


	/* search special case 'periodic': replace the duns-surfnr in nBlock[b].neighbor[j] with the periodic blocknr and adapt nBlock[b].map[j][] */
	if(compareStrings(dat[n], "periodic")>0)
	{
          if(blockbou[bouNr].surfs!=2)
	  {
	    printf(" ERROR: periodic BC needs a set with 2 lines or surfaces, but stored are %d in set:%s\n",blockbou[bouNr].surfs, blockbou[bouNr].name);
	    return;
	  }
          if(nBlock[blockbou[bouNr].nBlock[0]].dim==2)
	  {
		  j=blockbou[bouNr].side[0];   // a periodic bc has exactly 2 sides [0,1]
		  b=blockbou[bouNr].nBlock[0];
		  nBlock[b].neighbor[j]=nBlock[blockbou[bouNr].nBlock[1]].geo;
		  if(blockbou[bouNr].side[0]==0)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==1)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==2)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==3)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else { printf("ERROR: side:%d not known\n", blockbou[bouNr].side[0]); return; }
		  
		  j=blockbou[bouNr].side[1];
		  b=blockbou[bouNr].nBlock[1];
		  nBlock[b].neighbor[j]=nBlock[blockbou[bouNr].nBlock[0]].geo;
                  // Remark below 0 and 1 are switched compared to the above block 
		  if(blockbou[bouNr].side[1]==0)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==1)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==2)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==3)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
	          else { printf("ERROR: side:%d not known\n", blockbou[bouNr].side[1]); return; }
	  }
	  else  // 3D
	  {
	    printf(" WARNING: periodic BC only partly implemented for 3D:\n");
	    printf("   All bodies must have been generated by sweeping a 2D model at once into 3D\n");
	    //return;

		  j=blockbou[bouNr].side[0];   // a periodic bc has exactly 2 sides [0,1]
		  b=blockbou[bouNr].nBlock[0];
		  nBlock[b].neighbor[j]=nBlock[blockbou[bouNr].nBlock[1]].geo;
		  /*
                    blockbou[bouNr].side[0] stores the body surface index
                     x: surf0 at u=0
                     x: surf1 at umax
                   4 : surf2 at w=0
                   2 : surf3 at v=0
                   5 : surf4 at wmax
                   3 : surf5 at vmax

                    dir 1 from 3 (0,1,2) or 'u'('i' in duns manual) must be generated by sweeping of surf0, so nBlock[b].map[j][0] matches always 1
                    adjacent blocks can ony exist at surf2-5
                    mapping of side nr from 3D to 2D
                    3D  2D
                   2   0
                   3   1
                   4   2
                   5   3
		   */
		  printf("block side0:%d side1:%d\n", blockbou[bouNr].side[0], blockbou[bouNr].side[1]);
		  if(blockbou[bouNr].side[0]==2)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 2:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==3)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 2:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==4)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 2:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==5)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 2:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else { printf("ERROR: side:%d not known\n", blockbou[bouNr].side[0]); return; }
		  
		  j=blockbou[bouNr].side[1];
		  b=blockbou[bouNr].nBlock[1];
		  nBlock[b].neighbor[j]=nBlock[blockbou[bouNr].nBlock[0]].geo;
                  // Remark below 0 and 1 are switched compared to the above block 
		  if(blockbou[bouNr].side[1]==2)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 2:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==3)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 2:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==4)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 2:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==5)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 2:
		      nBlock[b].map[j][1]=6;
		      nBlock[b].map[j][2]=2;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 3:
		      nBlock[b].map[j][1]=3;
		      nBlock[b].map[j][2]=5;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 4:
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
		      nBlock[b].map[j][0]=1;
                    break;
                    case 5:
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=6;
		      nBlock[b].map[j][0]=1;
                    break;
		    }
		  }
	          else { printf("ERROR: side:%d not known\n", blockbou[bouNr].side[1]); return; }
	  }

          bouNr--;
	}

        n+=2;
      }
      bouNr++;

      descalNodes ( anz->n, node, scale);
      write2duns( setname, anz, node, e_enqire, lcase, nBlock, bouNr, blockbou );
      scalNodes ( anz->n, node, scale);

      /* free  */
      for(i=0; i<bouNr; i++) free(blockbou[i].surf); free(blockbou);
      for(i=0; i<length; i++) free(dat[i]); free(dat);
    }
  }
  else if (compare( format, "foam", 3)== 3)
  {
    length= strsplt( record, ' ', &dat);
    descalNodes ( anz->n, node, scale);
    if(length>2) write2foam( setname, length-2, &dat[2], anz, node, face, e_enqire, set, lcase );
    scalNodes ( anz->n, node, scale);

    /* free dat */
    for(i=0; i<length; i++) free(dat[i]); free(dat);
  }
  else if (compare( format, "isaac", 3)== 3)
  {
    length= strsplt( record, ' ', &dat);

    for( b=0; b<anz->b; b++)
    {
      if(nBlock[b].dim==2)
      {
        s=nBlock[b].geo;
        for(j=0; j<surf[s].nl; j++) strcpy(nBlock[b].bctype[j], "not defined");
      }
      if(nBlock[b].dim==3)
      {
        s=nBlock[b].geo;
        for(j=0; j<body[s].ns; j++) strcpy(nBlock[b].bctype[j], "not defined");
      }
    }
    
    /* add the type of boundary to the grid-blocks */
    bouNr=-1;
    n=2; while(n<length)
    {
      /* search the corresponding blocks */
      bset=getSetNr(dat[n+1]);
      if(bset<0)
      {
        printf("boundary set:%s does not exist\n", dat[n]);
        return;
      }
      bouNr++;
      if((blockbou=(NodeBlockbou *)realloc((NodeBlockbou *)blockbou, (bouNr+1)*sizeof(NodeBlockbou)) )==NULL)
      { printf(" ERROR: realloc failure, blockbou:%d could not be allocated\n\n", bouNr ); return; }
      strcpy(blockbou[bouNr].name,set[bset].name);
      blockbou[bouNr].surf=NULL;
      blockbou[bouNr].nBlock=NULL;
      blockbou[bouNr].side=NULL;
      blockbou[bouNr].surfs=0;

      if(nBlock[b].dim==2)
      {
       /* add the lcmb to the set */
       for(i=0; i<set[bset].anz_l; i++)
       {
        for(j=0; j<anzGeo->c; j++) if( lcmb[j].name != (char *)NULL )
        {
          for(l=0; l<lcmb[j].nl; l++)
	  {
	    printf("add lcmb:%s to set %s\n", lcmb[j].name, set[bset].name);
            if(lcmb[j].l[l]==set[bset].line[i]) seta(bset, "c", j);
	  }
	}
       }
      }

      /* look if a member of the set is used by a block */
      for( b=0; b<anz->b; b++)
      {
        if(nBlock[b].dim==2)
        {
          s=nBlock[b].geo;
          for(j=0; j<surf[s].nl; j++)
          {
            if(nBlock[b].map[j][0]==-1)   // has no neighbor
            {
              if((blockbou[bouNr].surf=(int *)realloc((int *)blockbou[bouNr].surf, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
              { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
              if((blockbou[bouNr].nBlock=(int *)realloc((int *)blockbou[bouNr].nBlock, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
              { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
              if((blockbou[bouNr].side=(int *)realloc((int *)blockbou[bouNr].side, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
              { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }

              l=surf[s].l[nBlock[b].bcface[j]];
              typ=surf[s].typ[nBlock[b].bcface[j]];
		
              if(typ=='l')
              {
                for(i=0; i<set[bset].anz_l; i++)
                {
                  if(l==set[bset].line[i])
                  {
                    printf(" store line %s in set:%s\n",line[l].name ,dat[n]);
                    strcpy(nBlock[b].bctype[j], dat[n]);
                    printf("set:%s btype:%s bouNr:%d line[%d]:%s block:%d side[%d]:%d\n",set[bset].name, dat[n],bouNr,blockbou[bouNr].surfs, line[set[bset].line[i]].name, nBlock[b].geo+1, j, nBlock[b].neighbor[j]);
                    blockbou[bouNr].surf[blockbou[bouNr].surfs] = nBlock[b].neighbor[j];
                    strcpy(blockbou[bouNr].bctype, dat[n]);
                    blockbou[bouNr].nBlock[blockbou[bouNr].surfs] = b;
                    blockbou[bouNr].side[blockbou[bouNr].surfs] = j;
                    blockbou[bouNr].surfs++;
                    break;
                  }
                }
              }
              if(typ=='c')
              {
                for(i=0; i<set[bset].anz_c; i++)
                {
                  if(l==set[bset].lcmb[i])
                  {
                    printf(" store lcmb %s in set:%s \n",lcmb[l].name, dat[n]);
                    strcpy(nBlock[b].bctype[j], dat[n]);
                    printf("set:%s btype:%s bouNr:%d lcmb[%d]:%s block:%d side:%d\n",set[bset].name, dat[n],bouNr,blockbou[bouNr].surfs, lcmb[set[bset].lcmb[i]].name, nBlock[b].geo+1, nBlock[b].neighbor[j]);
                    blockbou[bouNr].surf[blockbou[bouNr].surfs] = nBlock[b].neighbor[j];
                    strcpy(blockbou[bouNr].bctype, dat[n]);
                    blockbou[bouNr].nBlock[blockbou[bouNr].surfs] = b;
                    blockbou[bouNr].side[blockbou[bouNr].surfs] = j;
                    blockbou[bouNr].surfs++;
                    break;
                  }
                }
              }
	    }
          }
        }
        if(nBlock[b].dim==3)
	{
          v=nBlock[b].geo;
          for(i=0; i<set[bset].anz_s; i++)
          {
            for(j=0; j<body[v].ns; j++)
            {
              if(nBlock[b].map[j][0]==-1)   // has no neighbor
	      {
                s=body[v].s[nBlock[b].bcface[j]];
                if(s==set[bset].surf[i])
                {
                  printf(" store surf %s in set:%s\n", surf[s].name, dat[n]);
                  strcpy(nBlock[b].bctype[j], dat[n]);

		  if((blockbou[bouNr].surf=(int *)realloc((int *)blockbou[bouNr].surf, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                  { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
                  if((blockbou[bouNr].nBlock=(int *)realloc((int *)blockbou[bouNr].nBlock, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                  { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }
                  if((blockbou[bouNr].side=(int *)realloc((int *)blockbou[bouNr].side, (blockbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
                  { printf(" ERROR: realloc failure, blockbou:%d surface could not be allocated\n\n", bouNr ); return; }

		    printf("body:%s bsurf:%s surf:%s\n", body[v].name, surf[body[v].s[nBlock[b].bcface[j]]].name, surf[set[bset].surf[i]].name);
                  printf("set:%s btype:%s bouNr:%d surf[%d]:%s block:%d duns-side[%d]:%d\n",set[bset].name, dat[n],bouNr,blockbou[bouNr].surfs, surf[set[bset].surf[i]].name, nBlock[b].geo+1, j, nBlock[b].neighbor[j]);
                  blockbou[bouNr].surf[blockbou[bouNr].surfs] = nBlock[b].neighbor[j];
                  strcpy(blockbou[bouNr].bctype, dat[n]);
                  blockbou[bouNr].nBlock[blockbou[bouNr].surfs] = b;
                  blockbou[bouNr].side[blockbou[bouNr].surfs] = j;
                  blockbou[bouNr].surfs++;
                  break;
                }
              }
            }
          }
        }
      }

      /* search special case 'periodic': replace the cfd-surfnr in nBlock[b].neighbor[j] with the periodic blocknr and adapt nBlock[b].map[j][] */
      if(compareStrings(dat[n], "periodic")>0)
      {
          if(blockbou[bouNr].surfs!=2)
	  {
	    printf(" ERROR: periodic BC needs a set with 2 lines or surfaces, but stored are %d in set:%s\n",blockbou[bouNr].surfs, blockbou[bouNr].name);
	    return;
	  }
          if(nBlock[blockbou[bouNr].nBlock[0]].dim==2)
	  {
		  j=blockbou[bouNr].side[0];   // a periodic bc has exactly 2 sides side[0,1]
		  b=blockbou[bouNr].nBlock[0];
		  nBlock[b].neighbor[j]=nBlock[blockbou[bouNr].nBlock[1]].geo;
		  printf(" periodic BC, block:%d side:%d block:%d side:%d\n",nBlock[b].geo+1,blockbou[bouNr].side[0],nBlock[b].neighbor[j],blockbou[bouNr].side[0]);
		  if(blockbou[bouNr].side[0]==0)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==1)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==2)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[0]==3)
		  {
                    switch(blockbou[bouNr].side[1])
		    {
                    case 0:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else { printf("ERROR: side:%d not known\n", blockbou[bouNr].side[0]); return; }
		  
		  j=blockbou[bouNr].side[1];
		  b=blockbou[bouNr].nBlock[1];
		  nBlock[b].neighbor[j]=nBlock[blockbou[bouNr].nBlock[0]].geo;
                  // Remark below 0 and 1 are switched compared to the above block 
		  if(blockbou[bouNr].side[1]==0)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==1)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==2)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
		  else if(blockbou[bouNr].side[1]==3)
		  {
                    switch(blockbou[bouNr].side[0])
		    {
                    case 0:
		      nBlock[b].map[j][0]=5;
		      nBlock[b].map[j][1]=1;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 1:
		      nBlock[b].map[j][0]=2;
		      nBlock[b].map[j][1]=4;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 2:
		      nBlock[b].map[j][0]=1;
		      nBlock[b].map[j][1]=2;
		      nBlock[b].map[j][2]=3;
                    break;
                    case 3:
		      nBlock[b].map[j][0]=4;
		      nBlock[b].map[j][1]=5;
		      nBlock[b].map[j][2]=3;
                    break;
		    }
		  }
	          else { printf("ERROR: side:%d not known\n", blockbou[bouNr].side[1]); return; }


		  // fill nBlock[b].strt2[edge][dim],  nBlock[b].end_2[edge][dim]
		  j=blockbou[bouNr].side[0];
		  b=blockbou[bouNr].nBlock[0];
		  s=blockbou[bouNr].side[1];
		  v=blockbou[bouNr].nBlock[1];
		  for(i=0; i<3;i++)
		  {
		    //printf(" a%d:%s e%d:%s d%d, strt1:%d end1:%d\n",b,surf[nBlock[b].geo].name,j,line[surf[nBlock[b].geo].l[j]].name, i+1,  nBlock[b].strt1[j][i],nBlock[b].end_1[j][i]);
		    //printf(" a%d:%s e%d:%s d%d, strt1:%d end1:%d\n",v,surf[nBlock[v].geo].name,s,line[surf[nBlock[v].geo].l[s]].name, i+1, nBlock[v].strt1[s][i],nBlock[v].end_1[s][i]);
		    if(nBlock[v].map[s][i]>3)
		    {
		      nBlock[b].strt2[j][i]=nBlock[v].end_1[s][i];
		      nBlock[b].end_2[j][i]=nBlock[v].strt1[s][i];
		    }
		    else
		    {
		      nBlock[b].strt2[j][i]=nBlock[v].strt1[s][i];
		      nBlock[b].end_2[j][i]=nBlock[v].end_1[s][i];
		    }
		    //printf(" periodic BC b1:%d e%d d%d, b2:%d e:%d d%d strt2:%d end2:%d\n",b,j,nBlock[b].map[j][i], v,s,i+1, nBlock[b].strt2[j][i],nBlock[b].end_2[j][i]);
		  }
		  
		  anz->c++;
		  anz->c++;
	  }
	  else  // 3D
	  {
	    printf(" ERROR: periodic BC not implemented for 3D\n");
	    return;
	  }

	bouNr--;
      }
      
      n+=2;
    }
    bouNr++;

    for( b=0; b<anz->b; b++)
    {
      if(nBlock[b].dim==2)
      {
        s=nBlock[b].geo;
        for(j=0; j<surf[s].nl; j++)
        {
          if((surf[s].typ[nBlock[b].bcface[j]]=='l')&&(nBlock[b].bctype[j][0]==0))
          {
            printf(" WARNING: free surface:%s are not defined as boundary:%s in block: %d\n",line[surf[s].l[nBlock[b].bcface[j]]].name,nBlock[b].bctype[j], nBlock[b].geo+1);
          } 
          if((surf[s].typ[nBlock[b].bcface[j]]=='c')&&(nBlock[b].bctype[j][0]==0))
          {
            printf(" WARNING: free surface:%s are not defined as boundary:%s in block: %d\n",lcmb[surf[s].l[nBlock[b].bcface[j]]].name,nBlock[b].bctype[j], nBlock[b].geo+1);
          } 
	}
      }
      if(nBlock[b].dim==3)
      {
        s=nBlock[b].geo;
        for(j=0; j<body[s].ns; j++)
        {
          if(nBlock[b].bctype[j][0]==0)
          {
            printf(" WARNING: free surface:%s are not defined as boundary:%s in block: %d\n",surf[body[s].s[nBlock[b].bcface[j]]].name,nBlock[b].bctype[j], nBlock[b].geo+1);
          } 
	}
      }
    }

    descalNodes ( anz->n, node, scale);   
    write2isaac( setname, anz, node, e_enqire, lcase, nBlock, bouNr, blockbou );
    scalNodes ( anz->n, node, scale);

    /* free  */
    for(i=0; i<bouNr; i++) free(blockbou[i].surf); free(blockbou);
    for(i=0; i<length; i++) free(dat[i]); free(dat);
  }
  else if (compare( format, "stl", 3)== 3)
  {
    if(compareStrings( setname, "all" )<=0)
    {
      printf("ERROR: netgen-format was requested for a subset:%s. Up to now only the faces of the full mesh can be written (set all)\n",setname);
      return;
    }
    if(anz->f<1)
    {
      printf("ERROR: no faces exist\n");
      return;
    }

    descalNodes ( anz->n, node, scale);

    handle = fopen ("mesh.stl", "w+b");
    if (handle==NULL) { printf ("\nThe output file \"mesh.stl\" could not be opened.\n\n"); return; }
    printf (" write surface-mesh data in stl format \n");

    fprintf( handle, "solid\n");
    for (i=0; i<anz->f; i++)
    {
      if (face[i].type == 7)
      {
        v_result( &node[face[i].nod[0]].nx, &node[face[i].nod[1]].nx, v1);
        v_result( &node[face[i].nod[1]].nx, &node[face[i].nod[2]].nx, v2);
        v_result( &node[face[i].nod[2]].nx, &node[face[i].nod[0]].nx, v3);
        if(v_betrag(v1)==0) continue;
        if(v_betrag(v2)==0) continue;
        if(v_betrag(v3)==0) continue;
        v_prod( v1, v2, v3 );
        if(v_norm( v3, v1 )==0) continue;
        fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
        fprintf( handle, "    outer loop\n");
        for(k=0; k<3; k++)
        {
          fprintf( handle, "      vertex %e %e %e\n",
          node[face[i].nod[k]].nx,
          node[face[i].nod[k]].ny,
          node[face[i].nod[k]].nz);
        }
        fprintf( handle, "    endloop\n");
        fprintf( handle, "  endfacet\n");
      }
      else if (face[i].type == 8)
      {
        for(n=0; n<4; n++)
        {
          v_result( &node[face[i].nod[tr6[n][0]]].nx, &node[face[i].nod[tr6[n][1]]].nx, v1);
          v_result( &node[face[i].nod[tr6[n][1]]].nx, &node[face[i].nod[tr6[n][2]]].nx, v2);
          v_result( &node[face[i].nod[tr6[n][2]]].nx, &node[face[i].nod[tr6[n][0]]].nx, v3);
          if(v_betrag(v1)==0) continue;
          if(v_betrag(v2)==0) continue;
          if(v_betrag(v3)==0) continue;
          v_prod( v1, v2, v3 );
          if(v_norm( v3, v1 )==0) continue;
          fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
          fprintf( handle, "    outer loop\n");
          for(k=0; k<3; k++)
          {
            fprintf( handle, "      vertex %e %e %e\n",
            node[face[i].nod[tr6[n][k]]].nx,
            node[face[i].nod[tr6[n][k]]].ny,
            node[face[i].nod[tr6[n][k]]].nz);
          }
          fprintf( handle, "    endloop\n");
          fprintf( handle, "  endfacet\n");
        }
      }
      else if (face[i].type == 9)
      {
        for(n=0; n<2; n++)
        {
          v_result( &node[face[i].nod[qu4[n][0]]].nx, &node[face[i].nod[qu4[n][1]]].nx, v1);
          v_result( &node[face[i].nod[qu4[n][1]]].nx, &node[face[i].nod[qu4[n][2]]].nx, v2);
          v_result( &node[face[i].nod[qu4[n][2]]].nx, &node[face[i].nod[qu4[n][0]]].nx, v3);
          if(v_betrag(v1)==0) continue;
          if(v_betrag(v2)==0) continue;
          if(v_betrag(v3)==0) continue;
          v_prod( v1, v2, v3 );
          if(v_norm( v3, v1 )==0) continue;
          fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
          fprintf( handle, "    outer loop\n");
          for(k=0; k<3; k++)
          {
            fprintf( handle, "      vertex %e %e %e\n",
            node[face[i].nod[qu4[n][k]]].nx,
            node[face[i].nod[qu4[n][k]]].ny,
            node[face[i].nod[qu4[n][k]]].nz);
          }
          fprintf( handle, "    endloop\n");
          fprintf( handle, "  endfacet\n");
        }
      }
      else if (face[i].type ==10)
      {
        for(n=0; n<8; n++)
        {
          v_result( &node[face[i].nod[qu8[n][0]]].nx, &node[face[i].nod[qu8[n][1]]].nx, v1);
          v_result( &node[face[i].nod[qu8[n][1]]].nx, &node[face[i].nod[qu8[n][2]]].nx, v2);
          v_result( &node[face[i].nod[qu8[n][2]]].nx, &node[face[i].nod[qu8[n][0]]].nx, v3);
          if(v_betrag(v1)==0) continue;
          if(v_betrag(v2)==0) continue;
          if(v_betrag(v3)==0) continue;
          v_prod( v1, v2, v3 );
          if(v_norm( v3, v1 )==0) continue;
          fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
          fprintf( handle, "    outer loop\n");
          for(k=0; k<3; k++)
          {
            fprintf( handle, "      vertex %e %e %e\n",
            node[face[i].nod[qu8[n][k]]].nx,
            node[face[i].nod[qu8[n][k]]].ny,
            node[face[i].nod[qu8[n][k]]].nz);
          }
          fprintf( handle, "    endloop\n");
          fprintf( handle, "  endfacet\n");
        }
      }
    }
    fprintf( handle, "endsolid\n");
    fclose(handle);

    /* write netgen Edges */

    /* write the elements if they are be2 */
    j=0;
    for (i=0; i<anz->e; i++) if (e_enqire[e_enqire[i].nr].type == 11) j++;
    if(j)
    {
      handle = fopen ("mesh.ned", "w+b");
      if (handle==NULL) { printf ("\nThe output file \"mesh.ned\" could not be opened.\n\n"); return; }
      printf (" write edge data in ng format \n");
      fprintf( handle, "%d\n",j);
      printf("write %d edges\n", j);

      for (i=0; i<anz->e; i++)
      {
        if (e_enqire[e_enqire[i].nr].type == 11)
        {
          fprintf( handle, "2 %e %e %e %e %e %e\n",
          node[e_enqire[e_enqire[i].nr].nod[0]].nx,
          node[e_enqire[e_enqire[i].nr].nod[0]].ny,
          node[e_enqire[e_enqire[i].nr].nod[0]].nz,
          node[e_enqire[e_enqire[i].nr].nod[1]].nx,
          node[e_enqire[e_enqire[i].nr].nod[1]].ny,
          node[e_enqire[e_enqire[i].nr].nod[1]].nz);
        }
      }
      fclose(handle);
    }

    scalNodes ( anz->n, node, scale);
  }
  else
  {
    /* assume 'format' is  a second set for the mpc's */
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" ERROR: set:%s does not exist\n", setname);
      return;
    }
    i=getSetNr(format);
    if (i>-1)
    {
       length = sscanf(record, "%*s%*s%s%s%s%s", format, type, val[0], buffer);
       if (compare(type, "cycmpcf", 7)==7) cycmpc(setNr, i, "abqf", val[0], buffer);
       else if (compare(type, "cycmpc", 6)==6) cycmpc(setNr, i, format, val[0], buffer);
       else if (compare(type, "gap", 3)==3) gap(record);
       else if (compare(type, "areampc", 7)==7) { areampc(setNr, i, format, type, val[0], buffer, 0, node, 1, 1); }
       else
       { 
         errMsg(" ERROR: mpc type %s not yet supported, available are cycmpc or areampc\n", type );
         return;
       }
       /* the additional nodes for the HE20 (then HE26) elements need a new pos.  */
       adjustDrawNodes(0);
    }
    else errMsg(" ERROR: format:%s not yet supported, or set:%s does not exist\n", format, format );
  }
  printf (" ready\n");
}



/*------------------------------------------------------------------*/
/* Netzdaten im frd, abaqus oder nastran format rausschreiben       */
/*------------------------------------------------------------------*/

/* syntax: 1,2-8,12, 2-8+2 */
void lcparser(char *string, int *lc)
{
  int i, i1=0, j, val,pos,inc;
  char abuf[134];

  j=0;
  pos = strlen(string);

  i=0;
  while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; i++;}
  abuf[i-i1]=0;
  printf("val:%s\n", abuf);
  val=atoi(abuf);
  j=val-1;
  lc[j]=1;
  for(; i<=pos; i++)
  {
    if(string[i]==',')
    {
      i++;
      i1=i;
      while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; i++;}
      abuf[i-i1]=0;
      i--;
      val=atoi(abuf);
      printf("val:%d\n",val );
      j=val-1;
      lc[j]=1;
    }
    else if(string[i]=='-')
    {
      i++;
      i1=i;
      while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; i++;}
      abuf[i-i1]=0;
      val=atoi(abuf);
      if(string[i]=='+')
      {
        i++;
        i1=i;
        while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; printf("i:%d %d abuf:%c\n", i,i1,abuf[i-i1]); i++;}
        abuf[i-i1]=0;
        inc=atoi(abuf);
      }
      else inc=1;
      i--;
      // printf("inc:%d\n", inc);
      printf("val:%d\n", val);
      for (;j<val;j+=inc) { lc[j]=1; }
    }
    else if(string[i]==0) printf("parsed\n"); 
    else
    {
      printf("ERROR: sign:%c not known\n", string[i]);
    }
  }
}



void write2lst( char *datout, Summen *anz, Nodes *node, Elements *elem)
{
  FILE *handle1;
  int  i,n=0;

  handle1 = fopen (datout, "w+b");
  if (handle1==NULL) 
  {
    printf ("\nThe output file \"%s\" could not be opened.\n\n",datout);
    return;
  }
  else  printf (" file %s opened\n",datout);

  printf ("\n write list(lst) file  \n");

  fprintf (handle1, "# nodes\n");
  for (i=0; i<anz->n; i++)
  {
    n++;
    fprintf (handle1, " %d", node[i].nr);
    if(n>6) { n=0; fprintf (handle1, "\n"); }
  }
  fprintf (handle1, "\n# elements\n");
  n=0;
  for (i=0; i<anz->e; i++)
  {
    n++;
    fprintf (handle1, " %d", elem[i].nr);
    if(n>6) { n=0; fprintf (handle1, "\n"); }
  }
  fprintf (handle1, "\n");
  fclose(handle1);
}



void senddata( char *setname, char *format, Summen *anz, Nodes *node, Elements *e_enqire, Datasets *lcase , Scale *scale, int compFlag, char **val)
{
  int   setNr;
  int  length, i, j, n, ipuf=0, lc,lct, comp;
  char prognam[MAX_LINE_LENGTH];
  Summen    anz2[1];
  Nodes     *node2;
  Elements  *elem2;
  Datasets *lcase2=NULL;
  static int   *isort=NULL;
  extern int compareInt();
  static int step_number2=0;
  FILE *handle;
  double dx,dy,dz, sum_l=0.;
  int  xl[26];
  int *e2=NULL;
#define MM_TO_M  0.001
#define DT_KELVIN_TO_C -273.15

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in senddata, set:%s does not exist\n", setname);
    return;
  }
  /* replace 'lc' by 'ds' */
  if(val!=NULL) for(i=0; i<2; i++) if(compare( val[i], "lc", 2)== 2) { val[i][0]='d'; val[i][1]='s'; }
  strcpy ( prognam, setname);

  /* set is a sequence-set, tabular format */
  if(set[setNr].type)
  {
    length= strlen ( setname );
    strcpy (&prognam[length], ".txt");
    handle = fopen (prognam, "w+b" );
    if (handle==NULL) { printf ("\nThe output file \"%s\" could not be opened.\n\n", prognam); return;}

    if (compare( format, "seq", 3) == 3)
    {
      for (n=0; n<set[setNr].anz_n; n++)
      {
        fprintf ( handle, "%d\n",set[setNr].node[n]);
      }
      fclose(handle);
    }

    /* 1D stress for darwin in tabular format */
    if (compare( format, "dar", 3) == 3)
    {
      fprintf (handle, "# X               sigma\n");
      fprintf ( handle, "%-12.5e %-12.5e\n", sum_l* scale->w  * MM_TO_M, lcase[cur_lc].dat[cur_entity][set[setNr].node[0]]);
      for (n=1; n<set[setNr].anz_n; n++)
      {
        dx=node[set[setNr].node[n]].nx - node[set[setNr].node[n-1]].nx;
        dy=node[set[setNr].node[n]].ny - node[set[setNr].node[n-1]].ny;
        dz=node[set[setNr].node[n]].nz - node[set[setNr].node[n-1]].nz;
        sum_l+=sqrt(dx*dx+dy*dy+dz*dz);

        fprintf ( handle, "%-12.5e %-12.5e\n", (sum_l* scale->w) * MM_TO_M, lcase[cur_lc].dat[cur_entity][set[setNr].node[n]] );
      }
      strcpy(parameter[0], prognam);
      write2stack(1, parameter);
      fclose(handle);
    }
    return;
  }

  /* check if the format is known, else return */
  if ( (compare( format, "frd", 3) == 3)||
       (compare( format, "nas", 3) == 3)||
       (compare( format, "tcg", 3) == 3)||
       (compare( format, "lst", 3) == 3)||
       (compare( format, "abq", 3) == 3)||
       (compare( format, "ans", 3) == 3)||
       (compare( format, "ast", 3) == 3)||
       (compare( format, "dar", 3) == 3)||
       (compare( format, "sam", 3) == 3)||
       (compare( format, "stl", 3) == 3) );
  else
  {
    printf (" ERROR: format not recognized:%s\n\n", format);
    return;
  }

  descalNodes ( anz->n, node, scale);

  /* if the parameter sur was specified, send only the faces (with dataset) */
  if((val!=NULL)&&(compare( val[0], "sur", 3)== 3))
  {
    if((val!=NULL)&&(compare( val[1], "ds", 2)== 2)) strcpy(val[0],val[1]);
    anz2->emax=0;
    anz2->emin=MAX_INTEGER;
    anz2->e = set[setNr].anz_f;
    for (i=0; i<13; i++)
      anz2->etype[i] = 0;

    if ( (elem2 = (Elements *)malloc( (set[setNr].anz_f+1) * sizeof(Elements))) == NULL )
    { printf(" ERROR: malloc failed in senddata\n\n") ; return; }
    if ( (isort = (int *)realloc( isort, (set[setNr].anz_f+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed: isort\n\n" ); 
    for( i=0; i<set[setNr].anz_f; i++) isort[i]=set[setNr].face[i];
    qsort( isort, set[setNr].anz_f, sizeof(int), (void *)compareInt );

    for (i=0; i<set[setNr].anz_f; i++)
    {
      elem2[i].nr      = anz->enext++;
      elem2[i].type    = face[isort[i]].type;
      elem2[i].group   = face[isort[i]].group;
      elem2[i].mat     = face[isort[i]].mat;
      elem2[i].attr    = 0;
      anz2->etype[elem2[i].type]++;
      
      if (anz2->emax<elem2[i].nr ) anz2->emax = elem2[i].nr;
      if (anz2->emin>elem2[i].nr ) anz2->emin = elem2[i].nr;
      ipuf=0;
      if (elem2[i].type == 7) ipuf = 3;  /* TRI3  */
      if (elem2[i].type == 8) ipuf = 6;  /* TRI6  */
      if (elem2[i].type == 9) ipuf = 4;  /* QUAD4 */
      if (elem2[i].type == 10) ipuf = 8; /* QUAD8 */
      if (elem2[i].type == 11) ipuf = 2; /* BEAM */
      if (elem2[i].type == 12) ipuf = 3; /* BEAM3 */
      if (ipuf==0)
      {
        if(printFlag) printf (" elem(%d) not a known type (%d)\n", elem2[i].nr, elem2[i].type);
      }
      else
      {
        if (compFlag)
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[i].nod[j]=face[isort[i]].nod[j];
            seta( setNr, "n", elem2[i].nod[j] );
          }
        }
        else
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[i].nod[j]=face[isort[i]].nod[j];
          }
        }
      }
    }
  }
  /* if the parameter sur was not specified, send the elements (with dataset) */
  else
  {
    if ( (elem2 = (Elements *)malloc( (set[setNr].anz_e+1) * sizeof(Elements))) == NULL )
    { printf(" ERROR: malloc failed in senddata\n\n") ; return; }
    if ( (isort = (int *)realloc( isort, (set[setNr].anz_e+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed: isort\n\n" ); 
    for( i=0; i<set[setNr].anz_e; i++) isort[i]=set[setNr].elem[i];
    qsort( isort, set[setNr].anz_e, sizeof(int), (void *)compareInt );

    anz2->emax=0;
    anz2->emin=MAX_INTEGER;
    anz2->e = 0;
    elem2[0].nr =0;
    for (i=0; i<set[setNr].anz_e; i++)
    {
      if((i)&&(isort[i]==isort[i-1])) continue;

      elem2[anz2->e].nr      = isort[i];
      elem2[anz2->e].type    = e_enqire[isort[i]].type;
      elem2[anz2->e].group   = e_enqire[isort[i]].group;
      elem2[anz2->e].mat     = e_enqire[isort[i]].mat;
      elem2[anz2->e].attr    = e_enqire[isort[i]].attr;
      
      if (anz2->emax<elem2[anz2->e].nr ) anz2->emax = elem2[anz2->e].nr;
      if (anz2->emin>elem2[anz2->e].nr ) anz2->emin = elem2[anz2->e].nr;
      ipuf=0;
      if (elem2[anz2->e].type == 1) ipuf = 8;  /* HEXA8 */
      if (elem2[anz2->e].type == 2) ipuf = 6;  /* PENTA6 */
      if (elem2[anz2->e].type == 3) ipuf = 4;  /* TET4 */
      if (elem2[anz2->e].type == 4) ipuf = 20; /* HEX20 */
      if (elem2[anz2->e].type == 5) ipuf = 15; /* PENTA15 */
      if (elem2[anz2->e].type == 6) ipuf = 10; /* TET10 */
      if (elem2[anz2->e].type == 7) ipuf = 3;  /* TRI3  */
      if (elem2[anz2->e].type == 8) ipuf = 6;  /* TRI6  */
      if (elem2[anz2->e].type == 9) ipuf = 4;  /* QUAD4 */
      if (elem2[anz2->e].type == 10) ipuf = 8; /* QUAD8 */
      if (elem2[anz2->e].type == 11) ipuf = 2; /* BEAM */
      if (elem2[anz2->e].type == 12) ipuf = 3; /* BEAM3 */
      if (ipuf==0)
      {
        if(printFlag) printf (" elem(%d) not a known type (%d)\n", elem2[anz2->e].nr, elem2[anz2->e].type);
      }
      else
      {
        if (compFlag)
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[anz2->e].nod[j]=e_enqire[elem2[anz2->e].nr].nod[j];
            seta( setNr, "n", elem2[anz2->e].nod[j] );
          }
        }
        else
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[anz2->e].nod[j]=e_enqire[elem2[anz2->e].nr].nod[j];
          }
        }
      }
      /* if parameter frdm change penta to hexa */
      if ( (format[3]=='m')&&(elem2[anz2->e].type==5) )
      {
        printf (" elem(%d) of type (%d) will be changed to hex20\n", elem2[anz2->e].nr, elem2[anz2->e].type);

        for(j=0; j<3; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j];
        }
        xl[j]= xl[j-1];
        for(j=4; j<7; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j-1];
        }
        xl[j]= xl[j-1];
        for(j=8; j<10; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j-2];
        }
        xl[j++]= elem2[anz2->e].nod[2];
        xl[j]= elem2[anz2->e].nod[8];
  
        for(j=12; j<14; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j];
        }
         xl[j++]= elem2[anz2->e].nod[5];
        xl[j]= elem2[anz2->e].nod[14];
  
        for(j=16; j<19; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j-7];
        }
        xl[j]= xl[j-1];
  
  
        for(j=0; j<12; j++)
        { 
           elem2[anz2->e].nod[j]=xl[j];
        }
        for(n=16; n<20; n++)
        { 
           elem2[anz2->e].nod[n]=xl[j];
          j++;
        }
        for(n=12; n<16; n++)
        { 
           elem2[anz2->e].nod[n]=xl[j];
          j++;
        }

	elem2[anz2->e].type=4;
      }
      anz2->etype[elem2[anz2->e].type]++;
      anz2->e++;
    }
  }

  /* nodes must follow the elements (seta n above!) */
  /* sort the nodenumbers */

  if ( (node2 = (Nodes *)malloc( (anz->nmax+1) * sizeof(Nodes))) == NULL )
    { printf(" ERROR: malloc failed in senddata\n\n") ; return; }

  if ( (isort = (int *)realloc( isort, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed: isort\n\n" ); 
  for( i=0; i<set[setNr].anz_n; i++) isort[i]=set[setNr].node[i];
  qsort( isort, set[setNr].anz_n, sizeof(int), (void *)compareInt  );

  anz2->nmax = 0;
  anz2->nmin = MAX_INTEGER;
  anz2->n = 0;;
  node2[0].nr =0;
  for (i=0; i<set[setNr].anz_n; i++)
  {
    if((i)&&(isort[i]==isort[i-1])) continue;

    node2[anz2->n].nr = isort[i];
    node2[isort[i]].nx = node[isort[i]].nx;
    node2[isort[i]].ny = node[isort[i]].ny;
    node2[isort[i]].nz = node[isort[i]].nz;
    if (anz2->nmax<node2[anz2->n].nr) anz2->nmax = node2[anz2->n].nr;
    if (anz2->nmin>node2[anz2->n].nr) anz2->nmin = node2[anz2->n].nr;
    anz2->n++;
  }

  strcpy( anz2->model, setname);
  anz2->l = anz->l;
  anz2->u = anz->u;
  anz2->p = anz->p;
  anz2->sets = anz->sets;
  anz2->uheader = anz->uheader;
  anz2->pheader = anz->pheader;

  if(printFlag) printf (" write file \n");
  length= strlen ( setname );

  if (compare( format, "lst", 3) == 3)
  {
    strcpy (&prognam[length], ".lst");
    write2lst( prognam, anz2, node2, elem2);
  }
  else if (compare( format, "frd", 3) == 3)
  {
    strcpy (&prognam[length], ".frd");

    /* prepare the Datasets only if ds is specified */
    if((val)&&((compare( val[0], "ds", 2)== 2)||(compare( val[0], "bin", 2)== 2)||(compare( val[0], "dbin", 3)== 3)))
    {
      if(val[0][strlen(val[0])-1]=='+') ipuf=2; else ipuf=0;
      if(compare( val[0], "ds", 2)== 2)
      {
        if(val[0][2]=='l') lc=anz->l;
        else lc=atoi(&val[0][2]);
      }
      else
      {
        if(val[0][3]=='l') lc=anz->l;
        else lc=atoi(&val[0][3]);
      }
      if(lc<0) lc=anz->l+lc;

      if(lc)
      {
        lc--;
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
        anz2->l = 1;
        if ( (lcase2 = (Datasets *)realloc((Datasets *)lcase2, 1 * sizeof(Datasets))) == NULL )
        { printf("\n\n ERROR: malloc failure\n\n" ); exit(1); }

        strcpy(lcase2[0].name, lcase[lc].name);
        lcase2[0].value= lcase[lc].value ;
        lcase2[0].npheader = lcase[lc].npheader;
        lcase2[0].irtype = lcase[lc].irtype;
        lcase2[0].step_number=++step_number2;
        lcase2[0].analysis_type=lcase[lc].analysis_type;
        strcpy(lcase2[0].dataset_name,lcase[lc].dataset_name );
        strcpy(lcase2[0].dataset_text,lcase[lc].dataset_text);
        strcpy(lcase2[0].analysis_name,lcase[lc].analysis_name);
        lcase2[0].pheader=lcase[lc].pheader;
        lcase2[0].ncomps=0;
        lcase2[0].menu=NULL;
        lcase2[0].ictype=NULL;
        lcase2[0].icind1=NULL;
        lcase2[0].icind2=NULL;
        lcase2[0].iexist=NULL;
        lcase2[0].max=NULL;         
        lcase2[0].min=NULL;         
        lcase2[0].nmax=NULL;        
        lcase2[0].nmin=NULL;        
        lcase2[0].dat=NULL;       
        lcase2[0].compName=NULL;
        lcase2[0].icname=NULL;

        if ( (e2 = (int *)realloc(e2,  (lcase[lc].ncomps+1) * sizeof(int))) == NULL )
          printf("\n\n ERROR: malloc failed\n\n") ;
        if(val[1][0]=='e')
        {
          for(i=0; i<lcase[lc].ncomps; i++) e2[i]=0;
          lcparser(&val[1][1], e2);
	}
        else
          for(i=0; i<lcase[lc].ncomps; i++) e2[i]=1;

        for(i=0; i<lcase[lc].ncomps; i++)
        {
          if(e2[i]==1)
          {
            lcase2[0].ncomps++;

            if ( (lcase2[0].nmax = (int *)realloc( (int *)lcase2[0].nmax, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].nmin = (int *)realloc( (int *)lcase2[0].nmin, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].max = (float *)realloc((float *)lcase2[0].max, lcase2[0].ncomps * sizeof(float))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].min = (float *)realloc((float *)lcase2[0].min, lcase2[0].ncomps * sizeof(float))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].dat = (float **)realloc((float **)lcase2[0].dat, lcase2[0].ncomps * sizeof(float *))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].compName = (char **)realloc((char **)lcase2[0].compName, lcase2[0].ncomps * sizeof(char *))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].icname = (char **)realloc((char **)lcase2[0].icname, lcase2[0].ncomps * sizeof(char *))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].menu = (int *)realloc((int *)lcase2[0].menu, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].ictype = (int *)realloc((int *)lcase2[0].ictype, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].icind1 = (int *)realloc((int *)lcase2[0].icind1, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].icind2 = (int *)realloc((int *)lcase2[0].icind2, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].iexist = (int *)realloc((int *)lcase2[0].iexist, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
          
            comp=lcase2[0].ncomps-1;
            if ( (lcase2[0].dat[comp] = (float *)malloc( (anz2->nmax+1) * sizeof(float))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );	               
            if ( (lcase2[0].compName[comp] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
              printf("\n\n ERROR: malloc failed\n\n" );
            if ( (lcase2[0].icname[comp] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
              printf("\n\n ERROR: malloc failed\n\n" );

            lcase2[0].max[comp]=lcase[lc].max[i];
            lcase2[0].min[comp]=lcase[lc].min[i];
            strcpy ( lcase2[0].compName[comp], lcase[lc].compName[i]);
            lcase2[0].menu[comp]   = lcase[lc].menu[i]  ;
            lcase2[0].ictype[comp] = lcase[lc].ictype[i];
            lcase2[0].icind1[comp] = lcase[lc].icind1[i];
            lcase2[0].icind2[comp] = lcase[lc].icind2[i];
            lcase2[0].iexist[comp] = lcase[lc].iexist[i];
            for(n=0; n<=anz2->nmax; n++) lcase2[0].dat[comp][n]=lcase[lc].dat[i][n];
          }
	}
	  // check the type of the components and if all components are available
          for(i=0; i<lcase2[0].ncomps; i++)
          {
	    // vector
	    if(lcase2[0].ictype[i]==2)
	    {
	      // not enough comps for a vector?
	      if(lcase2[0].ncomps<i+3)
	      {
		lcase2[0].ictype[i]=1;
	      }
	      // check the next two comps
	      else
	      {
		if((lcase2[0].ictype[i+1]!=2)||(lcase2[0].ictype[i+2]!=2))
		lcase2[0].ictype[i+1]=lcase2[0].ictype[i+2]=1;
	        i=i+2;
	      }
	    }
	    // tensor
	    if(lcase2[0].ictype[i]==4)
	    {
	      // not enough comps for a vector?
	      if(lcase2[0].ncomps<i+6)
	      {
		lcase2[0].ictype[i]=1;
	      }
	      // check the next 5 comps
	      else
	      {
		if((lcase2[0].ictype[i+1]!=4)||(lcase2[0].ictype[i+2]!=4)||(lcase2[0].ictype[i+3]!=4)||(lcase2[0].ictype[i+4]!=4)||(lcase2[0].ictype[i+5]!=4))
		lcase2[0].ictype[i+1]=lcase2[0].ictype[i+2]=lcase2[0].ictype[i+3]=lcase2[0].ictype[i+4]=lcase2[0].ictype[i+5]=1;
	        i=i+5;
	      }
	    }
	    // complex vec
	    if(lcase2[0].ictype[i]==12)
	    {
	      // not enough comps for a vector?
	      if(lcase2[0].ncomps<i+6)
	      {
		lcase2[0].ictype[i]=1;
	      }
	      // check the next 5 comps
	      else
	      {
		if((lcase2[0].ictype[i+1]!=12)||(lcase2[0].ictype[i+2]!=12)||(lcase2[0].ictype[i+3]!=12)||(lcase2[0].ictype[i+4]!=12)||(lcase2[0].ictype[i+5]!=12))
		lcase2[0].ictype[i+1]=lcase2[0].ictype[i+2]=lcase2[0].ictype[i+3]=lcase2[0].ictype[i+4]=lcase2[0].ictype[i+5]=1;
	        i=i+5;
	      }
	    }
	    // complex tens
	    if(lcase2[0].ictype[i]==14)
	    {
	      // not enough comps for a vector?
	      if(lcase2[0].ncomps<i+12)
	      {
		lcase2[0].ictype[i]=1;
	      }
	      // check the next 5 comps
	      else
	      {
		if((lcase2[0].ictype[i+1]!=14)||(lcase2[0].ictype[i+2]!=14)||(lcase2[0].ictype[i+3]!=14)||(lcase2[0].ictype[i+4]!=14)||(lcase2[0].ictype[i+5]!=14)
		||(lcase2[0].ictype[i+6]!=14)||(lcase2[0].ictype[i+7]!=14)||(lcase2[0].ictype[i+8]!=14)||(lcase2[0].ictype[i+9]!=14)||(lcase2[0].ictype[i+10]!=14)
		||(lcase2[0].ictype[i+11]!=14))
		lcase2[0].ictype[i+1]=lcase2[0].ictype[i+2]=lcase2[0].ictype[i+3]=lcase2[0].ictype[i+4]=lcase2[0].ictype[i+5]=
		lcase2[0].ictype[i+6]=lcase2[0].ictype[i+7]=lcase2[0].ictype[i+8]=lcase2[0].ictype[i+9]=lcase2[0].ictype[i+10]=
		lcase2[0].ictype[i+11]=1;
	        i=i+11;
	      }
	    }
	}
        write2frd( prognam, anz2, node2, elem2, lcase2, ipuf);

        if(lcase2[0].ncomps)
	{
          for(i=0; i<lcase2[0].ncomps; i++)
          {
            free( lcase2[0].icname[i]); free( lcase2[0].compName[i]); free(lcase2[0].dat[i]);
          }
          free( lcase2[0].menu);
          free( lcase2[0].ictype);
          free( lcase2[0].icind1);
          free( lcase2[0].icind2);
          free( lcase2[0].iexist);
          free( lcase2[0].max);         
          free( lcase2[0].min);         
          free( lcase2[0].nmax);        
          free( lcase2[0].nmin);        
          free( lcase2[0].dat);       
          free( lcase2[0].compName);
          free( lcase2[0].icname);
	}
      }
      else
      {
       for(lc=0; lc<anz->l; lc++)
       {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
       }
       if(compare( val[0], "bin", 2)== 2) write2frd( prognam, anz2, node2, elem2, lcase, 1);
       else if(compare( val[0], "dbin", 3)== 3) write2frd( prognam, anz2, node2, elem2, lcase, 3);
       else  write2frd( prognam, anz2, node2, elem2, lcase, ipuf);
      }
    }
    else
    {
      anz2->l = 0;
      write2frd( prognam, anz2, node2, elem2, lcase, 0);
    } 
  }
  else if (compare( format, "nas", 3) == 3)
  {
    /* prepare the Datasets only if ds is specified */
    if((val)&&(compare( val[0], "ds", 2)== 2))
    {
      for(lc=0; lc<anz->l; lc++)
      {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
      }
    }
    else     anz2->l = 0;

    /* in case a coordinate system for nodes regarding the DOFs was defined */
    for (i=0; i<anz2->n; i++) node2[node2[i].nr].pflag=0;
    if(nodeCsys)
    {
      for(j=0; j<nodeCsys; j++)
      {
        printf(" nodes of set:%s get csys:%d\n", set[nodeCsysSet[j]].name, nodeCsysNr[j]);
        for (i=0; i<set[nodeCsysSet[j]].anz_n; i++) node2[set[nodeCsysSet[j]].node[i]].pflag=nodeCsysNr[j];
      }
    }
    write2nas( prognam, anz2, node2, elem2, lcase);
  }
  else if (compare( format, "abq", 3) == 3)
  {
    if(val)
    {
     if(compare( val[0], "ds", 2)== 2)
     {
       if(val[0][strlen(val[0])-1]=='+') ipuf=2; else ipuf=0;
       lc=atoi(&val[0][2])-1;
       if(lc==-1)
       {
         printf("\n ERROR: No dataset nr was given, only one dataset at a time can be written.\n\n");
	 return;
       }
       /* check if the data of the specified lcase (Dataset) are already available */
       if (!lcase[lc].loaded)
       {
         if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
         {
           printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
           return;
         }
         calcDatasets( lc, anz, node, lcase );
         recompileEntitiesInMenu(lc);
       }
     }
     if((compare( val[0], "tmf", 3)== 3)||(compare( val[0], "sta", 3)== 3)||(compare( val[0], "crp", 3)== 3))
     {
      for(lc=0; lc<anz->l; lc++)
      {
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
      }
     }
    }

    /* write the equations for incompatible elements */
    for(i=0; i<sum_equSets; i++)
    {
      /* temporary change the element-type to exclude unwanted elements */
      for(j=0; j<anz->e; j++) e_enqire[e_enqire[j].nr].type+=100;
      for(j=0; j<set[indSet[i]].anz_e; j++) e_enqire[set[indSet[i]].elem[j]].type-=100;

      /* gen mpcs */
      rnam(depSet[i], &set[depSet[i]].name[1]);
      areampc(depSet[i], indSet[i], "abq", "areampc", "123", "c", 0, node, 1, 1);

      /* restore orig type */
      for(j=0; j<set[indSet[i]].anz_e; j++) e_enqire[set[indSet[i]].elem[j]].type+=100;
      for(j=0; j<anz->e; j++) e_enqire[e_enqire[j].nr].type-=100;
    } 

    write2aba( prognam, anz2, node2, elem2, lcase, val, ipuf);

    /* add the equations to the mesh-file */
    for(i=0; i<sum_equSets; i++)
    {
      sprintf(buffer,"cat %s.equ >> %s", set[depSet[i]].name, prognam);
      system(buffer);    
      sprintf(buffer,"rm -f %s.equ", set[depSet[i]].name);
      system(buffer);
      sprintf(buffer,"-%s", set[depSet[i]].name);
      rnam(depSet[i], buffer);
    }
  }
  else if (compare( format, "dar", 3) == 3)
  {
    /* prepare the Datasets only if ds is specified */
    if((val)&&(val[0][0]=='v')); 
    else if((val)&&(compare( val[0], "ds", 2)== 2))
    {
      lc=atoi(&val[0][2]);
      if(lc)
      {
        lc--;

        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }

        /* search for related temps */      
        //if(lc>0) { lct=lc-1; while((lcase[lct].step_number==lcase[lc].step_number)&&(lct>0)) lct--; lct++; }
        if(lc) { for (lct=lc-1; lct>=0; lct--) { if(lcase[lct].step_number!=lcase[lc].step_number) break; } lct++; }
        else lct=1;
        while((lct<anz->l)&&(lcase[lct].step_number==lcase[lc].step_number))
        {
          if( (compare( lcase[lct].name, "NDTEMP", 6) == 6)||( compare( lcase[lct].name, "TEMP", 4) == 4)||( compare( lcase[lct].name, "TT3D", 4) == 4))
          {
            if (!lcase[lc].loaded)
            {
              if( pre_readfrdblock(copiedNodeSets , lct, anz, node, lcase )==-1) 
              {
                printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lct+1); 
                return;
              }
              calcDatasets( lct, anz, node, lcase );
              recompileEntitiesInMenu(lct);
            }
          }
          lct++;
        }
      }
      else
      {
       for(lc=0; lc<anz->l; lc++)
       {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
       }
      }
    }
    else val=0;
    write2darwin( prognam, anz2, node2, elem2, lcase, val);
  }
  else if (compare( format, "ans", 3) == 3)
  {
    if(val)
    {
     if(compare( val[0], "ds", 2)== 2)
     {
       lc=atoi(&val[0][2])-1;
       if(lc==-1)
       {
         printf("\n ERROR: No dataset nr was given, only one dataset at a time can be written.\n\n");
	 return;
       }
       /* check if the data of the specified lcase (Dataset) are already available */
       if (!lcase[lc].loaded)
       {
         if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
         {
           printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
           return;
         }
         calcDatasets( lc, anz, node, lcase );
         recompileEntitiesInMenu(lc);
       }
     }
     if((compare( val[0], "tmf", 3)== 3)||(compare( val[0], "sta", 3)== 3)||(compare( val[0], "crp", 3)== 3))
     {
      for(lc=0; lc<anz->l; lc++)
      {
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
      }
     }
    }
    else { anz2->l = 0; }
    write2ansys( prognam, anz2, node2, elem2, lcase, val);
  }
  else if (compare( format, "ast", 3) == 3)
  {
    anz2->l = 0;
    write2aster( prognam, anz2, node2, elem2, set, lcase);  //TODD
  }
  else if (compare( format, "sam", 3) == 3)
  {
    anz2->l = 0;
    write2samcef( prognam, anz2, node2, elem2, lcase);
  }
  else if (compare( format, "tcg", 3) == 3)
  {
    anz2->l = 0;
    write2tochnog( prognam, anz2, node2, elem2, lcase);
  }
  else
    printf (" ERROR: Format not recognized");

  free(node2);
  free(elem2);
  scalNodes ( anz->n, node, scale );

  strcpy(parameter[0], prognam);
  write2stack(1, parameter);
}



void setMaterial_rgb(int col, float alpha)
{
  GLfloat mat_shininess; /* 0->128 Blankheit */
  GLfloat mat_specular[4]; /* ungerichtete reflektion*/
  GLfloat mat_diffuse[4];  /* gerichtete reflektion*/
  mat_shininess= 120.;    /* blankheit, wirkt nicht wg lichtrichtung */

  mat_specular[0]=MAT_SPEC;
  mat_specular[1]=MAT_SPEC;
  mat_specular[2]=MAT_SPEC;
  /*
  mat_specular[0]=entitycol[col].r;  //MAT_SPEC 
  mat_specular[1]=entitycol[col].g;
  mat_specular[2]=entitycol[col].b;
  */
  mat_specular[3]=alpha;          

  mat_diffuse[0]=entitycol[col].r;
  mat_diffuse[1]=entitycol[col].g;
  mat_diffuse[2]=entitycol[col].b;
  /*
  mat_diffuse[0]=0;
  mat_diffuse[1]=0;
  mat_diffuse[2]=0;
  */         
  mat_diffuse[3]=alpha;          

  /* Reflexionseigenschaften des Materials aufbringen */
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_diffuse);
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,   mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_specular);
}
