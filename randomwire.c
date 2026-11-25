/* EFRW screening utility. Not an impedance or radiation model.
 * References: https://sprott.physics.wisc.edu/technote/randwire.htm
 * https://udel.edu/~mm/ham/randomWire/indexOld.html
 */
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct { const char *name; double lo,hi; int on; } Band;
static Band b[]={{"160 m",1.8,2,0},{"80 m",3.5,4,0},{"60 m*",5.3305,5.4064,0},{"40 m",7,7.3,1},{"30 m",10.1,10.15,1},{"20 m",14,14.35,1},{"17 m",18.068,18.168,1},{"15 m",21,21.45,1},{"12 m",24.89,24.99,1},{"10 m",28,29.7,1},{"6 m",50,54,0}};
#define NB (sizeof b/sizeof b[0])
#define CAP 20000
#define EPS 1e-9
typedef struct {double lo,hi;} Range;
typedef struct {Range r; double length,margin; size_t band;} Candidate;
static double low=20,high=150,tol=5,k=468;
static int quarter=1;
static Range ranges[CAP], blocked[CAP];
static Candidate candidates[CAP];
static int selected(void){for(size_t i=0;i<NB;i++)if(b[i].on)return 1;return 0;}
static double qref(void){for(size_t i=0;i<NB;i++)if(b[i].on)return k/(2*b[i].lo);return 0;}
static int cmp(const void *a,const void *z){double x=((const Range*)a)->lo,y=((const Range*)z)->lo;return (x>y)-(x<y);}
/* Minimum fractional distance from any half-wave resonance across the band. */
static double clearance(double l,size_t i){
 double a=l*b[i].lo/k,z=l*b[i].hi/k;
 double n=fmax(1,ceil(a));
 if(n<=z)return 0;
 double upper=(n-z)/n, lower=INFINITY;
 n=floor(a);if(n>=1)lower=(a-n)/n;
 return fmin(upper,lower);
}
static double worst(double l,size_t *band){double m=INFINITY;for(size_t i=0;i<NB;i++)if(b[i].on){double d=clearance(l,i);if(d<m){m=d;*band=i;}}return m;}
static int calculate(void){
 size_t nr=0;int no=0;double t=tol/100,cursor=quarter?fmax(low,qref()):low;
 if(!selected()||cursor>=high)return 0;
 for(size_t i=0;i<NB;i++)if(b[i].on){
  int nmax=(int)floor(high*b[i].hi/(k*(1-t)));
  for(int n=1;n<=nmax;n++){double a=n*k/b[i].hi*(1-t),z=n*k/b[i].lo*(1+t);if(z<cursor||a>high)continue;if(nr==CAP)return -1;blocked[nr++]=(Range){a,z};}
 }
 qsort(blocked,nr,sizeof *blocked,cmp);
 for(size_t i=0;i<nr;i++){if(blocked[i].lo>cursor+EPS){if(no==CAP)return -1;ranges[no++]=(Range){cursor,fmin(high,blocked[i].lo)};}cursor=fmax(cursor,blocked[i].hi);if(cursor>=high)break;}
 if(cursor<high-EPS){if(no==CAP)return -1;ranges[no++]=(Range){cursor,high};}return no;
}
static int rankcmp(const void *a,const void *z){const Candidate *x=a,*y=z;if(x->margin!=y->margin)return (x->margin<y->margin)-(x->margin>y->margin);return (x->length>y->length)-(x->length<y->length);}
static void rank(int n){for(int j=0;j<n;j++){
 /* Within a permitted interval, clearance is the minimum of affine
  * functions of length, hence concave. Ternary search maximizes it. */
 double a=ranges[j].lo,z=ranges[j].hi;size_t band=0;
 for(int s=0;s<80;s++){double x=a+(z-a)/3,y=z-(z-a)/3;if(worst(x,&band)<worst(y,&band))a=x;else z=y;}
 double l=(a+z)/2,m=worst(l,&band);candidates[j]=(Candidate){ranges[j],l,m,band};
 }qsort(candidates,(size_t)n,sizeof *candidates,rankcmp);}
static void results(void){int n=calculate(),off=0,ch;if(n>0)rank(n);do{erase();if(LINES<12||COLS<65){mvprintw(0,0,"Resize to 65 x 12; Q returns.");}else{
 mvprintw(0,0,"EFRW candidates ranked by worst-band half-wave clearance");
 mvprintw(1,0,"Quarter-wave reference: %.2f ft (%s); margin >= %.2f%%",qref(),quarter?"enforced":"advisory",tol);
 mvprintw(2,0," Range (ft)          Best ft   Best m   Margin   Limit");
 if(n==0)mvprintw(4,0,"No candidates. Change bounds, bands, or tolerance.");
 if(n<0)mvprintw(4,0,"Internal capacity exceeded.");
 for(int j=0;j<LINES-7&&off+j<n;j++){Candidate c=candidates[off+j];mvprintw(3+j,0,"%8.2f-%-8.2f %8.3f %8.3f %7.2f%%  %s",c.r.lo,c.r.hi,c.length,c.length*.3048,c.margin*100,b[c.band].name);}
 mvprintw(LINES-3,0,"Model only: no SWR/efficiency prediction; verify installed wire.");mvprintw(LINES-2,0,"%d intervals | Up/Down scroll | Q or Enter returns",n>0?n:0);
 }refresh();ch=getch();if(ch==KEY_DOWN&&off+1<n)off++;if(ch==KEY_UP&&off)off--;}while(ch!='q'&&ch!='Q'&&ch!='\n'&&ch!=KEY_ENTER);}
static int number(const char *s,double *v){char *e;double x=strtod(s,&e);if(e==s||*e||!isfinite(x))return 0;*v=x;return 1;}
static void analyze(void){char s[64]={0};double l=71;erase();echo();curs_set(1);mvprintw(0,0,"Analyze wire length in feet [71]: ");int rc=getnstr(s,63);noecho();curs_set(0);if(rc==ERR)return;if((*s&&!number(s,&l))||l<=0||l>2000){mvprintw(2,0,"Invalid length. Press any key.");getch();return;}
 int ch;do{erase();if(LINES<20||COLS<65){mvprintw(0,0,"Resize to 65 x 20; Q returns.");}else{mvprintw(0,0,"EFRW analysis: %.3f ft / %.3f m",l,l*.3048);mvprintw(1,0,"Band       Clearance    Half-wave screen    Quarter-wave");int row=3;for(size_t i=0;i<NB;i++)if(b[i].on){double d=clearance(l,i);mvprintw(row++,0,"%-8s %9.2f%%    %-19s %s",b[i].name,d*100,d<=EPS?"RESONANCE":d+EPS<tol/100?"NEAR RESONANCE":"PASS",l<k/(2*b[i].lo)?"SHORT":"AT/ABOVE");}size_t limit=0;double m=worst(l,&limit);mvprintw(15,0,"Worst band: %s (%.2f%%); %s lowest-band quarter-wave",b[limit].name,m*100,l<qref()?"below":"at/above");mvprintw(17,0,"Screening only. PASS does not guarantee a match.");mvprintw(18,0,"Q or Enter returns.");}refresh();ch=getch();}while(ch!='q'&&ch!='Q'&&ch!='\n'&&ch!=KEY_ENTER);}
static void settings(void){char s[64];double v[4]={low,high,tol,k};int valid=1;const char *p[]={"Minimum feet (0.1-2000)","Maximum feet (0.1-2000)","Tolerance percent (0-20)","Half-wave constant ft MHz (300-600)"};erase();echo();curs_set(1);for(int i=0;i<4;i++){s[0]=0;mvprintw(i*2,0,"%s [%.3f]: ",p[i],v[i]);if(getnstr(s,63)==ERR||(*s&&!number(s,&v[i])))valid=0;}noecho();curs_set(0);if(valid&&v[0]>=.1&&v[1]>v[0]&&v[1]<=2000&&v[2]>=0&&v[2]<=20&&v[3]>=300&&v[3]<=600){low=v[0];high=v[1];tol=v[2];k=v[3];}else{mvprintw(9,0,"Invalid settings; unchanged. Press any key.");getch();}}
static int tests(void){for(size_t i=0;i<NB;i++)b[i].on=0;b[5].on=1;low=20;high=40;tol=0;quarter=0;int n=calculate();if(n!=2||fabs(ranges[0].hi-468/14.35)>1e-8||fabs(ranges[1].lo-468.0/14)>1e-8)return 1;puts("20 m interval regression: PASS");if(clearance(468/14.2,5)>EPS||fabs(clearance(20,5)-(1-20*14.35/468))>EPS)return 1;puts("Full-band clearance: PASS");low=1;quarter=1;n=calculate();if(n!=2||fabs(ranges[0].lo-468.0/28)>EPS)return 1;puts("Quarter-wave floor: PASS");for(size_t i=0;i<NB;i++)b[i].on=(i>=3&&i<=9);low=20;high=150;tol=5;n=calculate();if(n!=4)return 1;rank(n);size_t limit=0;if(worst(71,&limit)<.05)return 1;for(int j=0;j<n;j++){Candidate c=candidates[j];if(c.margin+EPS<.05||c.length<c.r.lo-EPS||c.length>c.r.hi+EPS||(j&&c.margin>candidates[j-1].margin+EPS))return 1;}puts("40-10 m: four intervals, 71 ft passes, ranking: PASS");for(int j=0;j<n;j++)printf("%.3f-%.3f ft: %.3f ft, %.3f%%, %s\n",candidates[j].r.lo,candidates[j].r.hi,candidates[j].length,candidates[j].margin*100,b[candidates[j].band].name);return 0;}
int main(int argc,char **argv){if(argc==2&&!strcmp(argv[1],"--test"))return tests();if(argc!=1){fprintf(stderr,"Usage: %s [--test]\n",argv[0]);return 2;}initscr();cbreak();noecho();keypad(stdscr,TRUE);curs_set(0);int pos=0,ch;do{erase();if(LINES<23||COLS<65){mvprintw(0,0,"Resize terminal to at least 65 x 23; Q quits.");refresh();ch=getch();continue;}mvprintw(0,0,"END-FED RANDOM WIRE CALCULATOR 0.2");for(size_t i=0;i<NB;i++){if((int)i==pos)attron(A_REVERSE);mvprintw(2+(int)i,0,"[%c] %-7s %8.4f - %.4f MHz",b[i].on?'X':' ',b[i].name,b[i].lo,b[i].hi);if((int)i==pos)attroff(A_REVERSE);}mvprintw(14,0,"Length %.2f - %.2f ft | Tolerance %.2f%% | K %.2f",low,high,tol,k);mvprintw(15,0,"*60 m: conservative envelope, not transmit privileges.");mvprintw(16,0,"Lowest-band quarter-wave: %.2f ft | Minimum: %s",qref(),quarter?"ON":"OFF");mvprintw(19,0,"Arrows Move | Space Toggle | A All | N None");mvprintw(20,0,"C Calculate | W Analyze wire | M Toggle minimum");mvprintw(21,0,"S Settings | Q Quit");refresh();ch=getch();switch(ch){case KEY_UP:pos=(pos+(int)NB-1)%(int)NB;break;case KEY_DOWN:pos=(pos+1)%(int)NB;break;case ' ':b[pos].on=!b[pos].on;break;case 'a':case 'A':case 'n':case 'N':for(size_t i=0;i<NB;i++)b[i].on=(ch=='a'||ch=='A');break;case 'm':case 'M':quarter=!quarter;break;case 's':case 'S':settings();break;case 'c':case 'C':case 'w':case 'W':if(selected()){if(ch=='c'||ch=='C')results();else analyze();}else{mvprintw(18,0,"Select at least one band. Press any key.");getch();}break;}}while(ch!='q'&&ch!='Q');endwin();return 0;}
