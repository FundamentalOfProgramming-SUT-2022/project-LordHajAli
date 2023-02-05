#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
char clipboard[1000],arm[1000000];
int indices[1000] ,words[1000] ,cnt=0 ,stars[1000][3],acnt=0,scnt=0;
int y,x,X,Y,vx,vy,lastCol,dummy;
int line[1000],cx,cy=1,pos=1,po=0,lines=1,width=10,height=10,saved=1,hasname;
char mode[20],filename[100];

int min(int a,int b){
	return (a<b)*a+(b<=a)*b;
}
int max(int a,int b){
	return (a>b)*a+(b>=a)*b;
}
void saveas(char dir1[],char dir2[]){
	FILE*fp1=fopen(dir1,"w");
	FILE*fp2=fopen(dir2,"r");
	char str[1000];
	while(fgets(str,1000,fp2)!=NULL){
		fputs(str,fp1);
	}
	fclose(fp2),fclose(fp1);
}
void scr(char dir[],int pos){
	FILE *fp=fopen(dir,"r");
	char str[1000];
	for(int i=1;i<pos;i++) fgets(str,1000,fp);
	for(int i=0;i<min(16,lines);i++){
		if(fgets(str,1000,fp)!=NULL){
			mvprintw(i,5,"%s",str);
		}
	}
	fclose(fp);
}
void updateLines(){
	FILE *fp=fopen("maram.txt","r");
	char str[1000];
	for(int i=0;i<lines;i++){
		if(fgets(str,1000,fp)==NULL) break;
		line[i]=strlen(str)-1;
	}
	fclose(fp);
}
void lineNum(){
	FILE *fp=fopen("maram.txt","r");
	char str[1000];
	lines=0;
	while(fgets(str,1000,fp)!=NULL) lines++;
	fclose(fp);
}
void undoCreator(char directory[]){
    char str[1000];
    char str2[1000];
    realpath(directory,str2);
    for(int i=0;i<strlen(str2);i++) if(str2[i]=='\\') str2[i]='/';
    char*p=strstr(str2,"root");
    for(int i=0;i<p-str2;i++) str[i]=str2[i];
    str[p-str2]='\0';
    strcat(str,directory);
    FILE * fp;
    fp = fopen(directory,"r+");
    FILE * tmp;
    tmp = fopen(str,"w+");
    while(fgets(str,sizeof(str),fp)!=NULL){
        fprintf(tmp,"%s",str);
    }
    rewind(fp);
    fclose(tmp);
}
void createFile(char Directory[]){
    char dir[1000]={'\0'};
    int temp1=0, temp2;
    for(int i=1; i<=strlen(Directory); i++){
        if(Directory[i]=='/'){
            temp2=i;
            for(int j=temp1;j<temp2;j++){
                dir[j]=Directory[j];
            }
            temp1=temp2;
            mkdir(dir,0777);
        }
    }
    FILE *fp;
    fp = fopen(Directory,"r");
    if(fp){
        move(Y+1,0);
		printw("File already exists! Press any key to continue:");
		getch();
        fclose(fp);
        return;
    }
    fp = fopen(Directory,"a+");
    fclose(fp);
}
void undo(char directory[]){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
        return;
    }
    char str[1000];
    char str2[1000];
    realpath(directory,str2);
    for(int i=0;i<strlen(str2);i++) if(str2[i]=='\\') str2[i]='/';
    char*p=strstr(str2,"root");
    for(int i=0;i<p-str2;i++) str[i]=str2[i];
    str[p-str2]='\0';
    strcat(str,directory);
    FILE *tmp;
    tmp = fopen(str,"r+");
    FILE *buf;
    buf = fopen("buf.txt","w+");
    while(fgets(str,sizeof(str),tmp)!=NULL){
        fprintf(buf,"%s",str);
    }
    rewind(tmp);
    rewind(buf);
    tmp = fopen(str,"w");
    while(fgets(str,sizeof(str),fp)!=NULL){
        fprintf(tmp,"%s",str);
    }
    rewind(fp);
    rewind(tmp);
    fp = fopen(directory,"w");
    while(fgets(str,sizeof(str),buf)!=NULL){
        fprintf(fp,"%s",str);
    }
    fclose(tmp);
    fclose(fp);
    fclose(buf);
    remove("buf.txt");
}
void insert(char text[],char directory[],int line,int pos){
    FILE *fp;
    fp = fopen(directory,"r+");
	if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
	undoCreator(directory);
    FILE *buf;
    buf = fopen("buf.txt","w");
    char str[1000];
    for(int i =0;i<line-1;i++){
        if(feof(fp)){
            fprintf(buf,"%c",'\n');
        }
        if(fgets(str,sizeof(str),fp)!=NULL){
             //fp becomes the pointer of line "line".
            fprintf(buf,"%s",str);
        }
    }
    if(feof(fp)){
		fprintf(buf,"%c",'\n');
	}
    for(int i =0;i<pos;i++){
        if(feof(fp)){
            fprintf(buf,"%c",' ');
        }
        if(!feof(fp)){
            char c = fgetc(fp);
            if(c == '\n'){
                for(int j =0;j<pos-i;j++) fprintf(buf,"%c",' ');
                break;
            }
            else
                fprintf(buf,"%c",c);
        }
    }
    for(int i=0;i<strlen(text);i++){
        if(text[i]=='\\'){
            if(i==strlen(text)-1) continue;
            if(text[i+1]=='\\'){
                fputc('\\',buf);
            }
            if(text[i+1]=='n'){
                fputc('\n',buf);
            }
            i++;
        }
        else{
            fputc(text[i],buf);
        }
    }
    while(fgets(str,sizeof(str),fp)!=NULL){
        fprintf(buf,"%s",str);
    }
    rewind(buf);
    rewind(fp);
    while(fgets(str,sizeof(str),buf)!=NULL){
        fprintf(fp,"%s",str);
    }
    fclose(fp);
    fclose(buf);
	remove(directory);
    rename("buf.txt",directory);
    return;
}
void removeFrom(char directory[],int line,int pos, char mode,int size){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    undoCreator(directory);
    FILE *buf;
    buf = fopen("buf.txt","w+");
    int location = 0; //shows the starting point of removing.
    char str[1000];
    for(int i =1;i<line;i++){
        fgets(str,sizeof(str),fp);
        location += strlen(str);
    }
    location += pos+(mode=='b')*(-1)*size;
    rewind(fp);
    for(int i =0;i<location;i++){
        char c = fgetc(fp);
        fputc(c,buf);
    }
    for(int i=0;i<size;i++){
        char c=fgetc(fp);
    }
    while(fgets(str,sizeof(str),fp)!=NULL){
        fprintf(buf,"%s",str);
    }
    rewind(buf);
    rewind(fp);
    fp = fopen(directory,"w+");
    while(fgets(str,sizeof(str),buf)!=NULL){
        fprintf(fp,"%s",str);
    }
    fclose(fp);
    fclose(buf);
    remove(directory);
    rename("buf.txt",directory);
    return;
}
void copy(char directory[],int line,int pos, char mode,int size){
    FILE *fp;
    char str[1000];
    int location=0;
    fp = fopen(directory,"r");
	if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    for(int i=1;i<line;i++){
    	fgets(str,sizeof(str),fp);
        location += strlen(str);
    }
    location+=pos+(mode=='b')*(-1)*(size);
    rewind(fp);
    for(int i=0;i<location;i++){
        char c =fgetc(fp);
    }
    for(int i=0;i<size;i++){
    	str[i] = fgetc(fp);
    }
    str[size] = '\0';
    strcpy(clipboard,str);
    clipboard[size] = '\0';
    fclose(fp);
}
void cut(char directory[],int line,int pos, char mode,int size){
	FILE*fp = fopen(directory,"r");
	if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
	fclose(fp);
    undoCreator(directory);
	copy(directory,line,pos,mode,size);
	removeFrom(directory,line,pos,mode,size);
}
void paste(char directory[],int line,int pos){
	FILE*fp = fopen(directory,"r");
	if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
	fclose(fp);
    undoCreator(directory);
    insert(clipboard,directory,line,pos);
}
void find(char text[],char directory[],int star,int all,int byword,int count,int at){
    int len = strlen(text)+1;
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    if(at){
        if(all || count){
			move(Y+1,0);
			printw("Invalid!, Press any key to continue:");
			getch();
			return;
        }
    }
    else{
        if(all && count){
            move(Y+1,0);
			printw("Invalid!, Press any key to continue:");
			getch();
			return;
        }
    }
    char str[1000];
    int index=0;
    int word=1;
    int flag =0;
    char c;
    char c2;
    while(!feof(fp)){
        for(int i=0;i<len-1;i++){
            str[i]=fgetc(fp);
        }
        str[len-1]='\0';
        if(!strcmp(str,text)){
            indices[cnt] = index;
            c = fgetc(fp);
            if(feof(fp)) flag =1;
            if(!index) stars[0][2]=1;
            stars[cnt][1] =1;
            if(c==' ' || c=='\n' || feof(fp)) stars[cnt][0] = 1;
            rewind(fp);
            for(int i=0;i<index-1;i++) fgetc(fp);
            c = fgetc(fp);
            if(c==' ' || c=='\n'){
                stars[cnt][2]=1;
                words[cnt] = word+1;
            }
            else words[cnt] = word;
            cnt++;
            if(flag) break;
        }
        rewind(fp);
        for(int i=0;i<index;i++) fgetc(fp);
        index++;
        c = fgetc(fp);
        c2 = fgetc(fp);
        if(feof(fp)) break;
        else fseek(fp,-1,SEEK_CUR);
        if(c!=' ' && c!='\n' && !feof(fp)){
            if(index>1){
                fseek(fp,-2,SEEK_CUR);
                c = fgetc(fp);
                if(c==' ' || c=='\n'){
                    word++;
                }
                c = fgetc(fp);
            }
        }
    }
	if(!hasname && !saved){
		fclose(fp);
		hasname=1;
		move(Y+1,0);
		printw("Save previous file, Enter file name: ");
		c='m';
		cnt=0;
		char str2[100];
		while(c!='\n'){
			getyx(stdscr,Y,X);
			c=getch();
			if(c==127){
				cnt--;
				str2[cnt]='\0';
				move(Y,X-1),delch();
			}
			else if(c!='\n'){
				str2[cnt]=c;
				cnt++;
				insch(c),move(Y,X+1);
			}
		}
		str2[cnt]='\0';
		char name[100];
		for(int i=0;i<strlen(str);i++) name[i]=str[i];
		name[strlen(str)]='\0';
		FILE *fp=fopen(name,"w");
		saveas(name,"maram.txt");
		saved=1;
	}
	filename[0]='\0';
	hasname=0;
    if(star==0){
        int cnt2=0;
        for(int i=0;i<cnt;i++){
            if(stars[i][0]){
                cnt2++;
            }
        }
        if(count){
            char ss[100];
            sprintf(ss,"%d",cnt2);
            strcat(arm,ss);
            acnt+=strlen(ss);
        }
        else{
            int cnt2=0;
            for(int i=0;i<cnt;i++){
                if(stars[i][0]){
                    cnt2++;
                }
            }
            if(cnt2==0){
                arm[acnt]='-', arm[acnt+1]='1' ,arm[acnt+2]='\n';
                acnt+=3;
            }
            if(all){
                if(byword){
                    int counter=0;
                    for(int i=0;i<cnt;i++){
                        if(stars[i][0]){
                            counter++;
                            char ss[100];
                            if(counter==cnt2){
                                sprintf(ss,"%d",words[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                sprintf(ss,"%d",words[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]=',' , arm[acnt+1]=' ';
                                acnt+=2;
                            }
                        }
                    }
                }
                else{
                    int counter=0;
                    for(int i=0;i<cnt;i++){
                        if(stars[i][0]){
                            counter++;
                            char ss[100];
                            if(counter==cnt2){
                                sprintf(ss,"%d",indices[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                sprintf(ss,"%d",indices[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]=',' , arm[acnt+1]=' ';
                                acnt+=2;
                            }
                        }
                    }
                }
            }
            else{
                if(byword){
                    if(at){
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][0]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==at) break;
                        }
                        char ss[100];
                        sprintf(ss,"%d",words[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][0]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==1) break;
                        }
                        char ss[100];
                        sprintf(ss,"%d",words[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
                else{
                    if(at){
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][0]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==at) break;
                        }
                        char ss[100];
                        sprintf(ss,"%d",indices[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][0]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==1) break;
                        }
                        char ss[100];
                        sprintf(ss,"%d",indices[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
            }
        }
    }
    else if(star==1){
        if(cnt==0){
            arm[acnt]='-', arm[acnt+1]='1' ,arm[acnt+2]='\n';
            acnt+=3;
        }
        if(count){
            char ss[100];
            sprintf(ss,"%d",cnt);
            strcat(arm,ss);
            acnt+=strlen(ss);
            arm[acnt]='\n';
            acnt++;
        }
        else{
            if(all){
                if(byword){
                    char ss[100];
                    for(int i=0;i<cnt-1;i++){
                        sprintf(ss,"%d",words[i]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]=',' , arm[acnt+1]=' ';
                        acnt+=2;
                    }
                    sprintf(ss,"%d",words[cnt-1]);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    arm[acnt]='\n';
                    acnt++;
                }
                else{
                    char ss[100];
                    for(int i=0;i<cnt-1;i++){
                        sprintf(ss,"%d",indices[i]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]=',' , arm[acnt+1]=' ';
                        acnt+=2;
                    }
                    sprintf(ss,"%d",indices[cnt-1]);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    arm[acnt]='\n';
                    acnt++;
                }
            }
            else{
                if(byword){
                    char ss[100];
                    if(at){
                        sprintf(ss,"%d",words[at-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        sprintf(ss,"%d",words[0]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
                else{
                    char ss[100];
                    if(at){
                        sprintf(ss,"%d",indices[at-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        sprintf(ss,"%d",indices[0]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
            }
        }
    }
    else if(star==2){
        int cnt2=0;
        for(int i=0;i<cnt;i++){
            if(stars[i][2]){
                cnt2++;
            }
        }
        if(count){
            char ss[100];
            sprintf(ss,"%d",cnt2);
            strcat(arm,ss);
            acnt+=strlen(ss);
            arm[acnt]='\n';
            acnt++;
        }
        else{
            char ss[100];
            if(all){
                if(byword){
                    int counter=0;
                    for(int i=0;i<cnt;i++){
                        if(stars[i][2]){
                            counter++;
                            if(counter==cnt2){
                                sprintf(ss,"%d",(words[i]));
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                sprintf(ss,"%d",words[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]=',' , arm[acnt+1]=' ';
                                acnt+=2;
                            }
                        }
                    }
                }
                else{
                    int counter=0;
                    for(int i=0;i<cnt;i++){
                        if(stars[i][2]){
                            counter++;
                            if(counter==cnt2){
                                sprintf(ss,"%d",indices[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                sprintf(ss,"%d",indices[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]=',' , arm[acnt+1]=' ';
                                acnt+=2;
                            }
                        }
                    }
                }
            }
            else{
                if(byword){
                    if(at){
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][2]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==at) break;
                        }
                        sprintf(ss,"%d",words[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][2]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==1) break;
                        }
                        sprintf(ss,"%d",words[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
                else{
                    if(at){
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][2]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==at) break;
                        }
                        sprintf(ss,"%d",indices[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        int cnt2=0;
                        int cnt3=0;
                        for(int i=0;i<cnt;i++){
                            if(stars[i][2]){
                                cnt3++;
                            }
                            cnt2++;
                            if(cnt3==1) break;
                        }
                        sprintf(ss,"%d",indices[cnt2-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
            }
        }
    }
	FILE*mar=fopen("maram.txt","w");
	fputs(arm,mar);
	fclose(mar);
	for(int i=pos;i<=min(lines,15+pos);i++){
		move(i-pos,0),clrtoeol();
	}
	pos=1,po=0;
	lineNum();
	updateLines();
	for(int i=pos;i<=min(lines,15+pos);i++){
		move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
	}
	scr("maram.txt",pos);
	move(0,5);
	getyx(stdscr,y,x);
    cnt=0;
    for(int i=0;i<1000;i++) indices[i]=0 ,words[i]=0, stars[i][0]=0 ,stars[i][1]=0 , stars[i][2]=0;
    fclose(fp);
}
void findNoprint(char text[],char directory[],int star,int all,int byword,int count,int at){
    int len = strlen(text)+1;
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        move(Y+1,0);
		printw("File doesn't exist!");
		getch();
		return;
    }
    if(at){
        if(all || count){
            move(Y+1,0);
			printw("Invalid!");
			getch();
			return;
        }
    }
    else{
        if(all && count){
            move(Y+1,0);
			printw("Invalid!");
			getch();
			return;
        }
    }
    char str[1000];
    int index=0;
    int word=1;
    int flag =0;
    char c;
    char c2;
    while(!feof(fp)){
        for(int i=0;i<len-1;i++){
            str[i]=fgetc(fp);
        }
        str[len-1]='\0';
        if(!strcmp(str,text)){
            indices[cnt] = index;
            c = fgetc(fp);
            if(feof(fp)) flag =1;
            if(!index) stars[0][2]=1;
            stars[cnt][1] =1;
            if(c==' ' || c=='\n' || feof(fp)) stars[cnt][0] = 1;
            rewind(fp);
            for(int i=0;i<index-1;i++) fgetc(fp);
            c = fgetc(fp);
            if(c==' ' || c=='\n'){
                stars[cnt][2]=1;
                words[cnt] = word+1;
            }
            else words[cnt] = word;
            cnt++;
            if(flag) break;
        }
        rewind(fp);
        for(int i=0;i<index;i++) fgetc(fp);
        index++;
        c = fgetc(fp);
        c2 = fgetc(fp);
        if(feof(fp)) break;
        else fseek(fp,-1,SEEK_CUR);
        if(c!=' ' && c!='\n' && !feof(fp)){
            if(index>1){
                fseek(fp,-2,SEEK_CUR);
                c = fgetc(fp);
                if(c==' ' || c=='\n'){
                    word++;
                }
                c = fgetc(fp);
            }
        }
    }
    fclose(fp);
}
void makeZero(){
    cnt=0;
    for(int i=0;i<1000;i++) indices[i]=0 ,words[i]=0, stars[i][0]=0 ,stars[i][1]=0 , stars[i][2]=0;
}
void insertIndex(char text[],char directory[],int index){
    FILE *fp;
    fp = fopen(directory,"r+");
    FILE *buf;
    buf = fopen("buf.tmp","w+");
    for(int i=0;i<index;i++){
        char c = fgetc(fp);
        fprintf(buf,"%c",c);
    }
    fprintf(buf,"%s",text);
    char str[1000];
    while(fgets(str,sizeof(str),fp)!=NULL){
        fprintf(buf,"%s",str);
    }
    rewind(buf);
    rewind(fp);
    while(fgets(str,sizeof(str),buf)!=NULL){
        fprintf(fp,"%s",str);
    }
    fclose(fp);
    fclose(buf);
    remove("buf.tmp");
    return;
    
}
void replace(char text1[],char text2[],char directory[],int star,int all,int at){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    if(all && at){
        move(Y+1,0);
		printw("Invalid!");
		getch();
		return;
    }
    findNoprint(text1,directory,star,0,0,0,0);
    int counter = cnt;
    int counter2=0;
    if(!counter){
        return;
    }
    makeZero();
    undoCreator(directory);
    if(at){
        findNoprint(text1,directory,star,0,0,0,at);
        for(int i=0;i<counter;i++){
            if(stars[i][star]){
            counter2++;
                if(counter2==at){
                    removeFrom(directory,0,indices[i],'f',strlen(text1));
                    insertIndex(text2,directory,indices[i]);
                    makeZero();
                    return;
                }
            }
        }
    }
    else if(!all){
        findNoprint(text1,directory,star,0,0,0,0);
        for(int i=0;i<counter;i++){
            if(stars[i][star]){
                removeFrom(directory,0,indices[i],'f',strlen(text1));
                insertIndex(text2,directory,indices[i]);
                makeZero();
                return;
            }
        }
    }
    else{
        while(counter){
            findNoprint(text1,directory,star,0,0,0,0);
            for(int i=0;i<counter;i++){
                if(stars[i][star]){
                    removeFrom(directory,0,indices[i],'f',strlen(text1));
                    insertIndex(text2,directory,indices[i]);
                    makeZero();
                    counter--;
                    break;
                }
                if(i==counter-1) counter=0;
            }
        }
    }
    fclose(fp);
}
void autoIndent(char directory[]){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        move(Y+1,0);
		printw("File doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    FILE *buf;
    buf = fopen("buf.tmp","w+");
    char str[1000];
    int tab=0;
    undoCreator(directory);
    int counter=0;
    while(fgets(str,sizeof(str),fp)!=NULL){
        int i=0;
        int holder=0;
        while(i<strlen(str)){
            while(str[holder]==' '&& holder<strlen(str)) holder++;
            if(str[i]=='\n' || str[i]=='\0') break;
            int braceCnt=0;
            i=holder;
            while(str[i]!= '{' && str[i]!='}'){
                i++;
                if(str[i]=='\n' || str[i]=='\0') break;
            }
            int holder2=i-1;
            for(int j=0;j<=i-1;j++){
                if(str[holder2]==' ') holder2--;
                else break;
            }
            if(holder<=holder2){
                for(int j=0;j<counter;j++) fputs("    ",buf);
            }
            for(int j=holder;j<holder2+1;j++) fputc(str[j],buf);
            if(holder<=holder2) fputc(' ',buf);
            if(str[i]=='{'){
                if(holder>holder2){
                    for(int j=0;j<counter;j++) fprintf(buf,"%s","    ");
                }
                counter++;
                fprintf(buf,"%s","{\n");
            }
            if(str[i]=='}'){
                counter--;
                if(str[i+1]!='\0'){
                    fprintf(buf,"%c",'\n');
                    for(int j=0;j<counter;j++) fprintf(buf,"%s","    ");
                    fputs("}\n",buf);
                }
                else{
                    fputs("\n",buf);
                    for(int j=0;j<counter;j++) fprintf(buf,"%s","    ");
                    fputs("}",buf);
                }
            }
            i++;
            holder=i;
        }
    }
    FILE *buf2;
    buf2 = fopen("buf2.tmp","w+");
    rewind(buf);
    while(fgets(str,sizeof(str),buf)!=NULL){
        int flag=0;
        for(int i=0;i<strlen(str);i++){
            if(str[i]!= '\n' && str[i]!=' '){
                flag=1;
                break;
            }
        }
        if(flag) fputs(str,buf2);
    }
    rewind(buf2);
    fp = fopen(directory,"w+");
    while(fgets(str,sizeof(str),buf2)!=NULL){
        fprintf(fp,"%s",str);
    }
    fclose(fp);
    fclose(buf);
    fclose(buf2);
}
int cntG;
void grep(char text[],char directory[],char option){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        strcat(arm,directory);
        acnt+=strlen(directory);
        arm[acnt]=' ';
        acnt++;
        strcat(arm,": File doesn't exist!\n");
		acnt+=strlen(": File doesn't exist!\n");
        return;
    }
    char str[1000];
    if(option == 'c'){
        while(fgets(str,sizeof(str),fp)!=NULL){
            if(strstr(str,text) != NULL){
                cntG++;
            }
        }
    }
    else if(option == 'l'){
        while(fgets(str,sizeof(str),fp)!=NULL){
            if(strstr(str,text) != NULL){
                cntG++;
            }
        }
        if(cntG){
            strcat(arm,directory);
            acnt+=strlen(directory);
            arm[acnt]='\n';
            acnt++;
        }
    }
    else if(option == 'a'){
        while(fgets(str,sizeof(str),fp)!=NULL){
            if(strstr(str,text) != NULL){
                strcat(arm,directory);
                acnt+=strlen(directory);
                arm[acnt+1]=' ',arm[acnt]=':';
                acnt+=2;
                strcat(arm,str);
                acnt+=strlen(str);
                cntG++;
            }
        }
    }
    fclose(fp);
}
void compare(char file1[],char file2[]){
    FILE *fp1;
    fp1 = fopen(file1,"r");
    if(!fp1){
        move(Y+1,0);
		printw("File 1 doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    FILE *fp2;
    fp2 = fopen(file2,"r");
    if(!fp2){
        move(Y+1,0);
		printw("File 2 doesn't exist! Press any key to continue:");
		getch();
		return;
    }
    char str1[1000];
    char str2[1000];
    char word1[100][100];
    char word2[100][100];
    int dif=1;
    int lineNo=1,counter1=0,counter2=0,counter=0,line1=0,line2=0;
	if(feof(fp1)){
        while(fgets(str2,sizeof(str2),fp2)!=NULL){
            counter++;
        }
        rewind(fp2);
        strcat(arm,">>>>>>>>> #1 - #");
        acnt+=strlen(">>>>>>>>> #1 - #");
        char ss[100];
        sprintf(ss,"%d",counter);
        strcat(arm,ss);
        acnt+=strlen(ss);
        strcat(arm," >>>>>>>>>\n");
        acnt+=strlen(" >>>>>>>>>\n");
        for(int i=0;i<counter;i++){
            fgets(str2,sizeof(str2),fp2);
            strcat(arm,str2);
            acnt+=strlen(str2);
        }
    }
    while(!feof(fp1)){
        if(feof(fp2)){
            int lineNo1=lineNo;
            while(fgets(str1,sizeof(str1),fp1)!=NULL){
                lineNo++;
            }
            rewind(fp1);
            for(int i=1;i<lineNo1;i++) fgets(str1,sizeof(str1),fp1);
            strcat(arm,"<<<<<<<<< #");
            acnt+=strlen("<<<<<<<<< #");
            char ss[100];
            sprintf(ss,"%d",lineNo1);
            strcat(arm,ss);
            acnt+=strlen(ss);
            strcat(arm," - #");
            acnt+=strlen(" - #");
            sprintf(ss,"%d",lineNo-1);
            strcat(arm,ss);
            acnt+=strlen(ss);
            strcat(arm," <<<<<<<<<\n");
            acnt+=strlen(" <<<<<<<<<\n");
            for(int i=lineNo1;i<lineNo;i++){
                fgets(str1,sizeof(str1),fp1);
                strcat(arm,str1);
                acnt+=strlen(str1);
            }
            arm[acnt]='\n';
            acnt++;
            break;
        }
        fgets(str1,sizeof(str1),fp1);
        fgets(str2,sizeof(str2),fp2);
        if(str1[strlen(str1)-1]=='\n') str1[strlen(str1)-1]='\0';
        if(str2[strlen(str2)-1]=='\n') str2[strlen(str2)-1]='\0';
        if(strcmp(str1,str2)){
            int j=0;
            for(int i=0;i<=strlen(str1);i++){
                if(str1[i]==' ' || str1[i]=='\0'){
                    word1[counter1][j]='\0';
                    counter1++;
                    j=0;
                }
                else{
                    word1[counter1][j]=str1[i];
                    j++;
                }
            }
            j=0;
            for(int i=0;i<=strlen(str2);i++){
                if(str2[i]==' ' || str2[i]=='\0'){
                    word2[counter2][j]='\0';
                    counter2++;
                    j=0;
                }
                else{
                    word2[counter2][j]=str2[i];
                    j++;
                }
            }
            if(abs(counter1-counter2)>0){
                int flag1=0;
                if(abs(counter1-counter2)==1){
                    for(int i=0;i<counter2*(counter1>counter2)+counter1*(counter2>counter1);i++){
                        if(strcmp(word1[i],word2[i])){
                            flag1=1;
                            break;
                        }
                    }
                }
                if(flag1){
                    strcat(arm,"===== #");
                    acnt+=strlen("===== #");
                    char ss[100];
                    sprintf(ss,"%d",lineNo);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    strcat(arm," =====\n");
                    acnt+=strlen(" =====\n");
                    strcat(arm,str1);
                    acnt+=strlen(str1);
                    arm[acnt]='\n';
                    acnt++;
                    strcat(arm,str2);
                    acnt+=strlen(str2);
                    arm[acnt]='\n';
                    acnt++;
                }
                else{
                    strcat(arm,"===== #");
                    acnt+=strlen("===== #");
                    char ss[100];
                    sprintf(ss,"%d",lineNo);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    strcat(arm," =====\n");
                    acnt+=strlen(" =====\n");
                    if(counter2>counter1){
                        for(int i=0;i<counter1;i++){
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        arm[acnt]='\n';
                        acnt++;
                        for(int i=0;i<counter1;i++){
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        strcat(arm,">>");
                        acnt+=strlen(">>");
                        strcat(arm,word2[counter2-1]);
                        acnt+=strlen(word2[counter2-1]);
                        strcat(arm,"<<");
                        acnt+=strlen("<<");
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        for(int i=0;i<counter2;i++){
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        strcat(arm,">>");
                        acnt+=strlen(">>");
                        strcat(arm,word1[counter1-1]);
                        acnt+=strlen(word1[counter1-1]);
                        strcat(arm,"<<");
                        acnt+=strlen("<<");
                        for(int i=0;i<counter2;i++){
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        arm[acnt]='\n';
                        acnt++;
                    }
                }
            }
            else{
                for(int i=0;i<counter1;i++){
                    if(strcmp(word1[i],word2[i]))dif++;
                }
                if(dif>1){
                    strcat(arm,"===== #");
                    acnt+=strlen("===== #");
                    char ss[100];
                    sprintf(ss,"%d",lineNo);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    strcat(arm," =====\n");
                    acnt+=strlen(" =====\n");
                    strcat(arm,str1);
                    acnt+=strlen(str1);
                    arm[acnt]='\n';
                    acnt++;
                    strcat(arm,str2);
                    acnt+=strlen(str2);
                    arm[acnt]='\n';
                    acnt++;
                }
                else{
                    int diff;
                    for(int i=0;i<counter1;i++){
                        if(strcmp(word1[i],word2[i])) diff=i;
                    }
                    strcat(arm,"===== #");
                    acnt+=strlen("===== #");
                    char ss[100];
                    sprintf(ss,"%d",lineNo);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    strcat(arm," =====\n");
                    acnt+=strlen(" =====\n");
                    for(int i=0;i<counter1;i++){
                        if(i!=diff){
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        else{
                            strcat(arm,">>");
                            acnt+=strlen(">>");
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            strcat(arm,"<< ");
                            acnt+=strlen("<< ");
                        }
                    }
                    arm[acnt]='\n';
                    acnt++;
                    for(int i=0;i<counter1;i++){
                        if(i!=diff){
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        else{
                            strcat(arm,">>");
                            acnt+=strlen(">>");
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            strcat(arm,"<< ");
                            acnt+=strlen("<< ");
                        }
                    }
                    arm[acnt]='\n';
                    acnt++;
                }
            }
        }
        counter1=0,counter2=0,dif=0,lineNo++;
    }
    while(!feof(fp2)){
        int lineNo1=lineNo;
        while(fgets(str2,sizeof(str2),fp2)!=NULL){
            lineNo++;
        }
        rewind(fp2);
        for(int i=1;i<lineNo1;i++) fgets(str2,sizeof(str2),fp2);
            strcat(arm,">>>>>>>>> #");
            acnt+=strlen(">>>>>>>>> #");
            char ss[100];
            sprintf(ss,"%d",lineNo1);
            strcat(arm,ss);
            acnt+=strlen(ss);
            strcat(arm," - #");
            acnt+=strlen(" - #");
            sprintf(ss,"%d",lineNo-1);
            strcat(arm,ss);
            acnt+=strlen(ss);
            strcat(arm," >>>>>>>>>\n");
            acnt+=strlen(" >>>>>>>>>\n");
        for(int i=lineNo1;i<lineNo;i++){
            fgets(str2,sizeof(str2),fp2);
            strcat(arm,str2);
            acnt+=strlen(str2);
        }
        arm[acnt]='\n';
        acnt++;
    }
}
void dirTree(char directory[],int depth,int tab){
    if(depth<-1){
		return;
    }
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if(!d) return;
    char path[1000];
    if(!depth) return;
    if(depth>-1){
        if(d){
            while((dir=readdir(d)) != NULL){
                if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
                    for(int i=0;i<tab-1;i++){
                        arm[acnt]=179,arm[acnt+1]=' ',arm[acnt+2]=' ';
                        acnt+=3;
                    }
                    if(tab){
                        arm[acnt]=195,arm[acnt+1]=196,arm[acnt+2]=196;
                        acnt+=3;
                    }
                    strcpy(path,directory);
                    strcat(path,"/");
                    strcat(path,dir->d_name);
                    strcat(arm,dir->d_name);
                    acnt+=strlen(dir->d_name);
                    arm[acnt]='\n';
                    acnt++;
                    dirTree(path,depth-1,tab+1);
                }
            }
            closedir(d);
        }
    }
    if(depth==-1){
        if(d){
            int subNum=0;
            while((dir=readdir(d)) != NULL){
                subNum++;
                if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
                    for(int i=0;i<tab-1;i++){
                        arm[acnt]=179,arm[acnt+1]=' ',arm[acnt+2]=' ';
                        acnt+=3;
                    }
                    if(tab){
                        arm[acnt]=195,arm[acnt+1]=196,arm[acnt+2]=196;
                        acnt+=3;
                    }
                    strcpy(path,directory);
                    strcat(path,"/");
                    strcat(path,dir->d_name);
                    strcat(arm,dir->d_name);
                    acnt+=strlen(dir->d_name);
                    arm[acnt]='\n';
                    acnt++;
                    dirTree(path,-1,tab+1);
                }
            }
            closedir(d);
        }
    }
}
int main(){
	FILE*fp=fopen("maram.txt","w");
	fclose(fp);
	strcpy(mode," NORMAL ");
	initscr();
	clear();
	move(0,5);
	refresh();
	noecho();
	start_color();
	init_pair(3,8,8);
	init_pair(2,COLOR_BLACK,COLOR_GREEN);
	init_pair(1,COLOR_BLACK,COLOR_CYAN);
	init_pair(4,COLOR_BLACK,COLOR_YELLOW);
	int normalLine;
	getyx(stdscr,y,x);
	attron(COLOR_PAIR(1));
	mvprintw(21,0,"%s",mode);
	attroff(COLOR_PAIR(1));
	attron(COLOR_PAIR(2));
	printw(" ");
	printw("%s",filename);
	if(!saved) printw("   "),printw("+");
	printw(" ");
	attroff(COLOR_PAIR(2));
	attron(COLOR_PAIR(3));
	getyx(stdscr,dummy,lastCol);
	for(int i=lastCol;i<=COLS;i++) printw(" ");
	attroff(COLOR_PAIR(3));
	normalLine=21;
	refresh();
	move(y,x);

	while(1){
		cntG=0;
		for(int i=0;i<1000;i++) arm[i]='\0';
		arm[0]='\0';
		makeZero();
		getyx(stdscr,y,x);
		move(y,x);
		getyx(stdscr,y,x);
		for(int i=pos;i<=min(lines,15+pos);i++) mvprintw(i-pos,0,"%3d",i);
		move(y,x);
		char c=getch();
		getyx(stdscr,y,x);
		updateLines();
		if(c>300 ||c<0);
		else if(!strcmp(mode," VISUAL ") && c=='d'){
			undoCreator("maram.txt");
			if(y+po>vy) removeFrom("maram.txt",vy+1,vx,'f',scnt);
			else if(y+po<vy) removeFrom("maram.txt",y+po+1,x-5,'f',scnt);
			else if(y+po==vy) removeFrom("maram.txt",vy+1,min(vx,x-5),'f',scnt);
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol();
			}
			move(y,x);
			updateLines();
			lineNum();
			while(pos>1) if(pos+15>=lines) pos--,po--;
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			if(y<lines) move(y,x);
			else move(lines-1,line[lines-1]+5);
			strcpy(mode," NORMAL ");
		}
		else if(!strcmp(mode," VISUAL ") && c=='t'){
			undoCreator("maram.txt");
			if(y+po>vy) cut("maram.txt",vy+1,vx,'f',scnt);
			else if(y+po<vy) cut("maram.txt",y+po+1,x-5,'f',scnt);
			else if(y+po==vy) cut("maram.txt",vy+1,min(vx,x-5),'f',scnt);
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol();
			}
			move(y,x);
			updateLines();
			lineNum();
			while(pos>1) if(pos+15>=lines) pos--,po--;
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			if(y<lines) move(y,x);
			else move(lines-1,line[lines-1]+5);
			strcpy(mode," NORMAL ");
		}
		else if(!strcmp(mode," VISUAL ") && c=='c'){
			if(y+po>vy) copy("maram.txt",vy+1,vx,'f',scnt);
			else if(y+po<vy) copy("maram.txt",y+po+1,x-5,'f',scnt);
			else if(y+po==vy) copy("maram.txt",vy+1,min(vx,x-5),'f',scnt);
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol();
			}
			move(y,x);
			updateLines();
			lineNum();
			while(pos>1) if(pos+15>=lines) pos--,po--;
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			if(y<lines) move(y,x);
			else move(lines-1,line[lines-1]+5);
			strcpy(mode," NORMAL ");
		}
		else if(!strcmp(mode," VISUAL ") && c=='\033'){
			getch();
			char a;
			a=getch();
			if(a=='A'){
				if(y>0 && line[po+y-1]+5>=x) move(y-1,x);
				else if(y>0) move(y-1,line[po+y-1]+5);
				else if(!y && pos>1){
					pos--;
					po--;
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					if(line[po+y]+5>=x) move(y,x);
					else move(y,line[po+y]+5);
				}
			}
			else if(a=='B'){
				if(y<min(15,lines-1) && line[po+y+1]+5>=x) move(y+1,x);
				else if(y<min(15,lines-1)) move(y+1,line[po+y+1]+5);
				else if(y==15 && pos+15<lines){
					pos++;
					po++;
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					if(line[po+y]+5>=x) move(y,x);
					else move(y,line[po+y]+5);
				}
			}
			else if(a=='D'){
				if(x>5) move(y,x-1);
			}
			else if(a=='C'){
				if(x<line[po+y]+5) move(y,x+1);
				else if(x==line[po+y]+5 && (y==lines-1 || (y==15 && pos+15==lines)) ) move(y,x+1);
			}
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			move(y,x);
			getyx(stdscr,y,x);
			scnt=0;
			if(y+po<vy){
				for(int i=y;i<=min(vy-po,15);i++){
					for(int j=5;j<=line[i+po]+4;j++){
						if(i==y && j<=x) continue;
						if(i==vy-po && j>vx+5) continue;
						char t=mvinch(i,j);
						delch();
						attron(A_STANDOUT);
						insch(t);
						attroff(A_STANDOUT);
						scnt++;
					}
				}
				scnt+=abs(y+po-vy);
			}
			if(y+po>vy){
				for(int i=y;i>=max(vy-po,0);i--){
					for(int j=5;j<=line[i+po]+5;j++){
						if(i==y && j>x) continue;
						if(i!=y && j==line[i+po]+5) continue;
						if(i==vy-po && j<=vx+5) continue;
						char t=mvinch(i,j);
						delch();
						attron(A_STANDOUT);
						insch(t);
						attroff(A_STANDOUT);
						scnt++;
					}
				}
				scnt+=abs(y+po-vy);
			}
			if(y+po==vy){
				for(int j=5;j<=line[y+po]+5;j++){
					if(j<min(vx+5,x)) continue;
					if(j>=max(vx+5,x)) continue;
					char t=mvinch(y,j);
					delch();
					attron(A_STANDOUT);
					insch(t);
					attroff(A_STANDOUT);
					scnt++;
				}
			}
			move(y,x);
		}
		else if(c=='\033' && strcmp(mode," VISUAL ")){
			getch();
			char a;
			a=getch();
			if(a=='A'){  
				if(y>0 && line[po+y-1]+5>=x) move(y-1,x);
				else if(y>0) move(y-1,line[po+y-1]+5);
				else if(!y && pos>1){
					pos--;
					po--;
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					if(line[po+y]+5>=x) move(y,x);
					else move(y,line[po+y]+5);
				}
			}
			else if(a=='B'){
				if(y<min(15,lines-1) && line[po+y+1]+5>=x) move(y+1,x);
				else if(y<min(15,lines-1)) move(y+1,line[po+y+1]+5);
				else if(y==15 && pos+15<lines){
					pos++;
					po++;
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					if(line[po+y]+5>=x) move(y,x);
					else move(y,line[po+y]+5);
				}
			}
			else if(a=='D'){
				if(x>5) move(y,x-1);
			}
			else if(a=='C'){
				if(x<line[po+y]+5) move(y,x+1);
				else if(x==line[po+y]+5 && (y==lines-1 || (y==15 && pos+15==lines)) ) move(y,x+1);
			}
		}
		else if(!strcmp(mode," INSERT "),(c==KEY_BACKSPACE || c==127)){
			saved=0;
			if(x>5){
				removeFrom("maram.txt",pos+y,x-5,'b',1);
				move(y,x-1),delch(),line[po+y]--;
			}
		}
		else if(c==9 && strcmp(mode," VISUAL ")){
			strcpy(mode," VISUAL ");
			getyx(stdscr,vy,vx);
			vy+=po,vx-=5;
		}
		else if(c==96){
			strcpy(mode," NORMAL ");
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			move(y,x);
		}
		else if(strcmp(mode," INSERT ") && c=='i'){
			strcpy(mode," INSERT ");
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			move(y,x);
		}
		else if(!strcmp(mode," NORMAL ") &&(c=='u' || c=='U')){
			saved=0;
			undo("maram.txt");
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol();
			}
			pos=1,po=0;
			lineNum();
			updateLines();
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			move(0,5);
			getyx(stdscr,y,x);
		}
		else if(!strcmp(mode," NORMAL ") &&(c==':')){
			char str[1000];
			int cnt=0;
			mvprintw(22,0,"%c",c);
			while(c!='\n'){
				getyx(stdscr,Y,X);
				c=getch();
				if(c==KEY_BACKSPACE || c==127){
					cnt--;
					str[cnt]='\0';
					move(Y,X-1),delch();
				}
				else if(c!='\n'){
					str[cnt]=c;
					cnt++;
					insch(c),move(Y,X+1);
				}
			}
			str[cnt]='\0';
			if(strstr(str,"saveas")){
				char name[100];
				hasname=1;
				for(int i=7;i<strlen(str);i++) name[i-7]=str[i];
				name[strlen(str)-7]='\0';
				FILE *fp=fopen(name,"w");
				saveas(name,"maram.txt");
				strcpy(filename,name);
				filename[strlen(name)]='\0';
				saved=1;
			}
			else if(strstr(str,"save")){
				if(!hasname){
					hasname=1;
					move(Y+1,0);
					printw("Enter file name: ");
					c='m';
					cnt=0;
					while(c!='\n'){
						getyx(stdscr,Y,X);
						c=getch();
						if(c==127){
							cnt--;
							str[cnt]='\0';
							move(Y,X-1),delch();
						}
						else if(c!='\n'){
							str[cnt]=c;
							cnt++;
							insch(c),move(Y,X+1);
						}
					}
					str[cnt]='\0';
					char name[100];
					for(int i=0;i<strlen(str);i++) name[i]=str[i];
					name[strlen(str)]='\0';
					FILE *fp=fopen(name,"w");
					saveas(name,"maram.txt");
					strcpy(filename,name);
					filename[strlen(name)]='\0';
					saved=1;
				}
				else{
					saved=1;
					saveas(filename,"maram.txt");
				}
			}
			else if(strstr(str,"ai")){
				undoCreator("maram.txt");
				autoIndent("maram.txt");
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol();
				}
				pos=1,po=0;
				lineNum();
				updateLines();
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
				}
				scr("maram.txt",pos);
				move(0,5);
				getyx(stdscr,y,x);
			}
			else if(strstr(str,"open")){
				if(!hasname && !saved){
					hasname=1;
					move(Y+1,0);
					printw("Save previous file, Enter file name: ");
					c='m';
					cnt=0;
					char str2[100];
					while(c!='\n'){
						getyx(stdscr,Y,X);
						c=getch();
						if(c==127){
							cnt--;
							str2[cnt]='\0';
							move(Y,X-1),delch();
						}
						else if(c!='\n'){
							str2[cnt]=c;
							cnt++;
							insch(c),move(Y,X+1);
						}
					}
					str2[cnt]='\0';
					char name[100];
					for(int i=0;i<strlen(str);i++) name[i]=str[i];
					name[strlen(str)]='\0';
					FILE *fp=fopen(name,"w");
					saveas(name,"maram.txt");
					saved=1;
				}
				char name[100];
				for(int i=5;i<strlen(str);i++) name[i-5]=str[i];
				name[strlen(str)-5]='\0';
				FILE *fp=fopen(name,"r");
				if(!fp) fp=fopen(name,"w");
				strcpy(filename,name);
				filename[strlen(name)]='\0';
				saveas("maram.txt",name);
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol();
				}
				pos=1,po=0;
				lineNum();
				updateLines();
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
				}
				scr("maram.txt",pos);
				move(0,5);
				getyx(stdscr,y,x);
			}
			else if(strstr(str,"createfile --file ")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				createFile(dir);
			}
			else if(strstr(str,"insertstr --file ")){
				char dir[200];
				char text[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--str");
				p+=6;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							text[i-1]='\0';
							break;
						}
						text[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							text[i]='\0';
							break;
						}
						text[i]=str[p-str+i];
					}
				}
				p=strstr(str,"---pos");
				p+=7;
				char maram[20];
				int line;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==':'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				line = atoi(maram);
				int index;
				p+=strlen(maram)+1;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]=='\0'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				index=atoi(maram);
				insert(text,dir,line,index);
			}
			else if(strstr(str,"removestr --file ")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--pos");
				p+=6;
				char maram[20];
				int line;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==':'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				line = atoi(maram);
				int index;
				p+=strlen(maram)+1;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]=='\0'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				index=atoi(maram);
				p=strstr(str,"-size");
				p+=6;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==' '){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				int size = atoi(maram);
				char direction;
				direction = str[strlen(str)-1];
				removeFrom(dir,line,index,direction,size);
			}
			else if(strstr(str,"copystr --file ")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--pos");
				p+=6;
				char maram[20];
				int line;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==':'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				line = atoi(maram);
				int index;
				p+=strlen(maram)+1;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==' '){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				index=atoi(maram);
				p=strstr(str,"-size");
				p+=6;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==' '){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				int size = atoi(maram);
				char direction;
				direction = str[strlen(str)-1];
				copy(dir,line,index,direction,size);
			}
			else if(strstr(str,"cutstr --file ")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--pos");
				p+=6;
				char maram[20];
				int line;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==':'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				line = atoi(maram);
				int index;
				p+=strlen(maram)+1;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==' '){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				index=atoi(maram);
				p=strstr(str,"-size");
				p+=6;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==' '){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				int size = atoi(maram);
				char direction;
				direction = str[strlen(str)-1];
				cut(dir,line,index,direction,size);
			}
			else if(strstr(str,"pastestr --file ")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--pos");
				p+=6;
				char maram[20];
				int line;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==':'){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				line = atoi(maram);
				int index;
				p+=strlen(maram)+1;
				for(int i=0;i<strlen(str);i++){
					if(str[p-str+i]==' '){
						maram[i]='\0';
						break;
					}
					maram[i]=str[p-str+i];
				}
				index=atoi(maram);
				paste(dir,line,index);
			}
			else if(strstr(str,"replace --str1 ")){
				char dir[200];
				char text[200];
				char text2[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--str1");
				p+=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							text[i-1]='\0';
							break;
						}
						text[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							text[i]='\0';
							break;
						}
						text[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--str2");
				p+=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							text2[i-1]='\0';
							break;
						}
						text2[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							text2[i]='\0';
							break;
						}
						text2[i]=str[p-str+i];
					}
				}
				int all=0;
				int at=0;
				if(strstr(str,"all")) all=1;
				if(p=strstr(str,"-at")){
					char *q = p+4;
					int i=0;
					char num[20];
					while(*(q+i)!=' '){
						num[i]=*(q+i);
						i++;
					}
					num[i]='\0';
					at = atoi(num);
				}
				int star;
				if(text[0]=='*'){
					star=0;
					for(int i=1;i<strlen(text);i++) text[i-1]=text[i];
					text[strlen(text)-1]='\0';
					mvprintw(5,5,"%s",text);
				}
				else if(text[strlen(text)-1]=='*') star=2,text[strlen(text)-1]='\0';
				else star=1;
				replace(text,text2,dir,star,all,at);
			}
			else if(strstr(str,"auto-indent")){
				char dir[200];
				char *p=strstr(str,"auto-indent");
				p +=12;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]=='\0'){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				autoIndent(dir);
			}
			else if(strstr(str,"undo --file")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]=='\0'){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				undo(dir);
			}
			else if(strstr(str,"undo")){
				saved=0;
				undo("maram.txt");
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol();
				}
				pos=1,po=0;
				lineNum();
				updateLines();
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
				}
				scr("maram.txt",pos);
				move(0,5);
				getyx(stdscr,y,x);
			}
			else if(strstr(str,"cat --file ")){
				char dir[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]=='\0'){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				FILE*fp=fopen(dir,"r");
				if(!fp){
					move(Y+1,0);
					printw("File doesn't exist, Press any key to continue: ");
					getch();
				}
				else if(!hasname && !saved){
					fclose(fp);
					hasname=1;
					move(Y+1,0);
					printw("Save previous file, Enter file name: ");
					c='m';
					cnt=0;
					char str2[100];
					while(c!='\n'){
						getyx(stdscr,Y,X);
						c=getch();
						if(c==127){
							cnt--;
							str2[cnt]='\0';
							move(Y,X-1),delch();
						}
						else if(c!='\n'){
							str2[cnt]=c;
							cnt++;
							insch(c),move(Y,X+1);
						}
					}
					str2[cnt]='\0';
					char name[100];
					for(int i=0;i<strlen(str);i++) name[i]=str[i];
					name[strlen(str)]='\0';
					FILE *fp=fopen(name,"w");
					saveas(name,"maram.txt");
					saved=1;
				}
				if(fp){
					filename[0]='\0';
					hasname=0;
					saveas("maram.txt",dir);
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol();
					}
					pos=1,po=0;
					lineNum();
					updateLines();
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					move(0,5);
					getyx(stdscr,y,x);
				}
			}
			else if(strstr(str,"find --str ")){
				char dir[200];
				char text[200];
				char *p =strstr(str,"--file");
				p +=7;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+i];
					}
				}
				p=strstr(str,"--str");
				p+=6;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							text[i-1]='\0';
							break;
						}
						text[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							text[i]='\0';
							break;
						}
						text[i]=str[p-str+i];
					}
				}
				int count=0;
				int all=0;
				int at=0;
				int byword=0;
				if(strstr(str,"-count")) count=1;
				if(strstr(str,"all")) all=1;
				if(strstr(str,"byword")) byword=1;
				if(p=strstr(str,"-at")){
					char *q = p+4;
					int i=0;
					char num[20];
					while(*(q+i)!=' '){
						num[i]=*(q+i);
						i++;
					}
					num[i]='\0';
					at = atoi(num);
				}
				int star;
				if(text[0]=='*'){
					star=0;
					for(int i=1;i<strlen(text);i++) text[i-1]=text[i];
					text[strlen(text)-1]='\0';
				}
				else if(text[strlen(text)-1]=='*') star=2,text[strlen(text)-1]='\0';
				else star=1;
				find(text,dir,star,all,byword,count,at);
			}
			else if(strstr(str,"grep ")){
				if(!hasname && !saved){
					hasname=1;
					move(Y+1,0);
					printw("Save previous file, Enter file name: ");
					c='m';
					cnt=0;
					char str2[100];
					while(c!='\n'){
						getyx(stdscr,Y,X);
						c=getch();
						if(c==127){
							cnt--;
							str2[cnt]='\0';
							move(Y,X-1),delch();
						}
						else if(c!='\n'){
							str2[cnt]=c;
							cnt++;
							insch(c),move(Y,X+1);
						}
					}
					str2[cnt]='\0';
					char name[100];
					for(int i=0;i<strlen(str);i++) name[i]=str[i];
					name[strlen(str)]='\0';
					FILE *fp=fopen(name,"w");
					saveas(name,"maram.txt");
					saved=1;
				}
				filename[0]='\0';
				hasname=0;
				cntG=0;
				char dir[200],text[200];
				char *p=strstr(str,"--str");
				p+=6;
				if(str[p-str]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+i]=='"' && str[p-str+i-1]!='\\'){
							text[i-1]='\0';
							break;
						}
						text[i-1]=str[p-str+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+i]==' '){
							text[i]='\0';
							break;
						}
						text[i]=str[p-str+i];
					}
				}
				char op;
				if(strstr(str,"-l")) op='l';
				else if(strstr(str,"-c")) op='c';
				else op='a';
				p =strstr(str,"--files");
				p +=8;
				int q=0;
				while(str[p-str+q]!='\0'){
					if(str[p-str+q]=='"'){
						for(int i=1;i<strlen(str);i++){
							if(str[p-str+q+i]=='"' && str[p-str+q+i-1]!='\\'){
								dir[i-1]='\0';
								break;
							}
							dir[i-1]=str[p-str+q+i];
						}
						q+=strlen(dir)+2;
					}
					else{
						for(int i=0;i<strlen(str);i++){
							if(str[p-str+q+i]==' '){
								dir[i]='\0';
								break;
							}
							dir[i]=str[p-str+q+i];
						}
						q+=strlen(dir)+1;
					}
					if(strcmp(dir,"-c") && strcmp(dir,"-l")) grep(text,dir,op);
					if(op=='l') cntG=0;
					if(str[p-str+q-1]=='\0') break;
				}
				FILE*fp=fopen("maram.txt","w");
				if(op=='c'){
					char ss[100];
					sprintf(ss,"%d",cntG);
					strcat(arm,ss);
					acnt+=strlen(ss);
					fputs(arm,fp);
					fclose(fp);
				}
				else if(op=='l'){
					fputs(arm,fp);
					fclose(fp);
				}
				else if(op=='a'){
					fputs(arm,fp);
					fclose(fp);
				}
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol();
				}
				for(int i=0;i<1000;i++)arm[i]='\0';
				acnt=0;
				pos=1,po=0;
				lineNum();
				updateLines();
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
				}
				scr("maram.txt",pos);
				move(0,5);
				getyx(stdscr,y,x);
			}
			else if(strstr(str,"compare")){
				char dir[200],dir2[200];
				char *p=strstr(str,"compare");
				p+=8;
				int q=0;
				if(str[p-str+q]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+q+i]=='"' && str[p-str+q+i-1]!='\\'){
							dir[i-1]='\0';
							break;
						}
						dir[i-1]=str[p-str+q+i];
					}
					q+=strlen(dir)+2;
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+q+i]==' '){
							dir[i]='\0';
							break;
						}
						dir[i]=str[p-str+q+i];
					}
					q+=strlen(dir)+1;
				}
				if(str[p-str+q]=='"'){
					for(int i=1;i<strlen(str);i++){
						if(str[p-str+q+i]=='"' && str[p-str+q+i-1]!='\\'){
							dir2[i-1]='\0';
							break;
						}
						dir2[i-1]=str[p-str+q+i];
					}
				}
				else{
					for(int i=0;i<strlen(str);i++){
						if(str[p-str+q+i]==' '){
							dir2[i]='\0';
							break;
						}
						dir2[i]=str[p-str+q+i];
					}
				}
				if(!hasname && !saved){
					hasname=1;
					move(Y+1,0);
					printw("Save previous file, Enter file name: ");
					c='m';
					cnt=0;
					char str2[100];
					while(c!='\n'){
						getyx(stdscr,Y,X);
						c=getch();
						if(c==127){
							cnt--;
							str2[cnt]='\0';
							move(Y,X-1),delch();
						}
						else if(c!='\n'){
							str2[cnt]=c;
							cnt++;
							insch(c),move(Y,X+1);
						}
					}
					str2[cnt]='\0';
					char name[100];
					for(int i=0;i<strlen(str);i++) name[i]=str[i];
					name[strlen(str)]='\0';
					FILE *fp=fopen(name,"w");
					saveas(name,"maram.txt");
					saved=1;
				}
				filename[0]='\0';
				hasname=0;
				compare(dir,dir2);
				FILE*fp=fopen("maram.txt","w");
				fputs(arm,fp);
				fclose(fp);
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol();
				}
				for(int i=0;i<1000;i++)arm[i]='\0';
				acnt=0;
				pos=1,po=0;
				lineNum();
				updateLines();
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
				}
				scr("maram.txt",pos);
				move(0,5);
				getyx(stdscr,y,x);
			}
			else if(strstr(str,"tree ")){
				char dep[10];
				int counter=0,depth;
				for(int i=5;i<strlen(str);i++){
					if(str[5]=='-'){
						if(str[6]==1) depth=-1;
						else depth=-2;
					}
					dep[counter]=str[i];
					counter++;
				}
				dep[counter]='\0';
				depth = atoi(dep);
				if(depth<-1){
					move(Y+1,0);
					printw("Invalid depth! Press any key to continue:");
					getch();
					continue;
				}
				else if(!hasname && !saved){
					hasname=1;
					move(Y+1,0);
					printw("Save previous file, Enter file name: ");
					c='m';
					cnt=0;
					char str2[100];
					while(c!='\n'){
						getyx(stdscr,Y,X);
						c=getch();
						if(c==127){
							cnt--;
							str2[cnt]='\0';
							move(Y,X-1),delch();
						}
						else if(c!='\n'){
							str2[cnt]=c;
							cnt++;
							insch(c),move(Y,X+1);
						}
					}
					str2[cnt]='\0';
					char name[100];
					for(int i=0;i<strlen(str);i++) name[i]=str[i];
					name[strlen(str)]='\0';
					FILE *fp=fopen(name,"w");
					saveas(name,"maram.txt");
					saved=1;
				}
				filename[0]='\0';
				hasname=0;
				if(depth>=-1){
					dirTree(".",depth,0);
					FILE*fp=fopen("maram.txt","w");
					fputs(arm,fp);
					fclose(fp);
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol();
					}
					for(int i=0;i<1000;i++)arm[i]='\0';
					acnt=0;
					pos=1,po=0;
					lineNum();
					updateLines();
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					move(0,5);
					getyx(stdscr,y,x);
				}
			}
			move(22,0);
			clrtoeol();
			move(23,0);
			clrtoeol();
			move(y,x);
		}
		else if(!strcmp(mode," NORMAL ") &&(c=='/')){
			char str[1000];
			int cnt1=0;
			mvprintw(22,0,"%c",c);
			while(c!='\n'){
				getyx(stdscr,Y,X);
				c=getch();
				if(c==KEY_BACKSPACE || c==127){
					cnt1--;
					str[cnt1]='\0';
					move(Y,X-1),delch();
				}
				else if(c!='\n'){
					str[cnt1]=c;
					cnt1++;
					insch(c),move(Y,X+1);
				}
			}
			str[cnt1]='\0';
			cnt=0;
			findNoprint(str,"maram.txt",1,0,0,0,0);
			int yx[100][2];
			for(int i=0;i<cnt;i++){
				for(int j=1;j<=lines;j++){
					if(indices[i]<=line[j-1]){
						yx[i][0]=j;
						break;
					}
					else indices[i]-=line[j-1]+1;
				}
				yx[i][1]=indices[i];
			}
			move(y,x);
			int counter=0,counter2=0,end=0,conat=0,s=0;
			while(cnt){
				int xx,yy,fla=1;
				getyx(stdscr,yy,xx);
				getyx(stdscr,y,x);
				for(int i=pos;i<=min(lines,15+pos);i++){
					move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
				}
				scr("maram.txt",pos);
				move(yy,xx);
				int nx,ny;
				for(int i=0;i<cnt;i++){
					if(yy+pos==yx[cnt-1][0] && xx-5==yx[cnt-1][1]) nx=yx[0][1],ny=yx[0][0];
					else if(yy+pos>yx[i][0]) break;
					else if(yy+pos<yx[i][0]) ny=yx[i][0],nx=yx[i][1];
					else if(yy+pos==yx[i][0]){
						if(xx-5<yx[i][1]) nx=yx[i][1];
						else break;
					}
				}
				for(int i=0;i<cnt;i++){
					if(yx[i][0]>=pos && yx[i][0]<=pos+15){
						for(int j=0;j<strlen(str);j++){
							char t= mvinch(yx[i][0]-pos,yx[i][1]+5+j);
							delch();
							attron(COLOR_PAIR(4));
							insch(t);
							attroff(COLOR_PAIR(4));
						}
					}
				}
				move(yy,xx);
				if(ny>=pos && ny<=pos+15) yy=ny-pos,xx=nx+5;
				else{
					if(ny<=lines-15) pos=ny,po=ny-1,yy=0,xx=nx+5;
					else if(ny>lines-15){
						pos=max(1,lines-15);
						po=pos-1;
						yy=ny-pos;
						xx=nx+5;
					}
				}
				char f=getch();
				if(f!='n' && f!='N') break;
			}
		}
		else if(!strcmp(mode," NORMAL ") &&(c=='=')){
			undoCreator("maram.txt");
			autoIndent("maram.txt");
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol();
			}
			pos=1,po=0;
			lineNum();
			updateLines();
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			move(0,5);
			getyx(stdscr,y,x);
		}
		else if(!strcmp(mode," NORMAL ") &&(c=='p')){
			undoCreator("maram.txt");
			paste("maram.txt",y+pos,x-5);
			getyx(stdscr,y,x);
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol();
			}
			move(y,x);
			updateLines();
			lineNum();
			while(pos>1) if(pos+15>=lines) pos--,po--;
			for(int i=pos;i<=min(lines,15+pos);i++){
				move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
			}
			scr("maram.txt",pos);
			move(y,x);
		}
		else if(!strcmp(mode," INSERT ") && c!=127){
			saved=0;
			if(c!='\n') insch(c);
			char text[2];
			text[0]=c,text[1]='\0';
			undoCreator("maram.txt");
			insert(text,"maram.txt",pos+y,x-5);
			if(c=='\n'){
				lines++;
				updateLines();
				if(y==15){
					pos++,po++;
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					move(y,5);
				}
				else{
					for(int i=pos;i<=min(lines,15+pos);i++){
						move(i-pos,0),clrtoeol() ,mvprintw(i-pos,0,"%3d",i);
					}
					scr("maram.txt",pos);
					move(y+1,5);
					cx=0,cy++;
				}
			}
			else{
				move(y,x+1);
				cx++;
				line[po+y]++;
			}
			updateLines();
		}

		refresh();
		getyx(stdscr,y,x);
		move(normalLine,0);
		clrtoeol();
		attron(COLOR_PAIR(1));
		mvprintw(21,0,"%s",mode);
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(2));
		printw(" ");
		printw("%s",filename);
		if(!saved) printw("   "),printw("+");
		printw(" ");
		attroff(COLOR_PAIR(2));
		attron(COLOR_PAIR(3));
		getyx(stdscr,dummy,lastCol);
		for(int i=lastCol;i<=COLS;i++) printw(" ");
		attroff(COLOR_PAIR(3));
		normalLine=21;
		move(y,x);
	}
	
}
