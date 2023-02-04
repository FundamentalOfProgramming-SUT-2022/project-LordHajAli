#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
char clipboard[1000],arm[1000000];
int indices[1000] ,words[1000] ,cnt=0 ,stars[1000][3],acnt=0;

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
        printf("File already exists!\n");
        fclose(fp);
        return;
    }
    fp = fopen(Directory,"a+");
    fclose(fp);
}

void insert(char text[],char directory[],int line,int pos){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        printf("File doesn't exist!\n");
        return;
    }
    undoCreator(directory);
    FILE *buf;
    buf = fopen("buf.tmp","w+");
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
    remove("buf.tmp");
    return;
}

void cat(char directory[]){
    FILE *fp;
    fp = fopen(directory,"r");
    char str[1000];
    while(!feof(fp)){
        char c=fgetc(fp);
        if(feof(fp)) break;
        printf("%c",c);
        arm[acnt]=c;
        acnt++;
    }
    printf("\n");
    arm[acnt]='\n';
    acnt++;
}
void removeFrom(char directory[],int line,int pos, char mode,int size){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        printf("Invalid directory!\n");
        return;
    }
    undoCreator(directory);
    FILE *buf;
    buf = fopen("buf.tmp","w+");
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
    remove("buf.tmp");
    return;
}
void copy(char directory[],int line,int pos, char mode,int size){
    FILE *fp;
    char str[1000];
    int location=0;
    fp = fopen(directory,"r");
    if(!fp){
        printf("File doesn't exist!\n");
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
    FILE* fp = fopen(directory,"r");
    if(!fp){
        printf("File doesn't exist!\n");
        return;
    }
    fclose(fp);
    undoCreator(directory);
	copy(directory,line,pos,mode,size);
	removeFrom(directory,line,pos,mode,size);
}
void paste(char directory[],int line,int pos){
    FILE* fp = fopen(directory,"r");
    if(!fp){
        printf("File doesn't exist!\n");
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
        printf("File doesn't exist!\n");
        return;
    }
    if(at){
        if(all || count){
            printf("Invalid!\n");
            return;
        }
    }
    else{
        if(all && count){
            printf("Invalid!\n");
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
    if(star==0){
        int cnt2=0;
        for(int i=0;i<cnt;i++){
            if(stars[i][0]){
                cnt2++;
            }
        }
        if(count){
            printf("%d\n",cnt2);
            char ss[100];
            sprintf(ss,"%d",cnt2);
            strcat(arm,ss);
            acnt+=strlen(ss);
            return;
        }
        else{
            int cnt2=0;
            for(int i=0;i<cnt;i++){
                if(stars[i][0]){
                    cnt2++;
                }
            }
            if(cnt2==0){
                printf("-1\n");
                arm[acnt]='-', arm[acnt+1]='1' ,arm[acnt+2]='\n';
                acnt+=3;
                return;
            }
            if(all){
                if(byword){
                    int counter=0;
                    for(int i=0;i<cnt;i++){
                        if(stars[i][0]){
                            counter++;
                            char ss[100];
                            if(counter==cnt2){
                                printf("%d\n",words[i]);
                                sprintf(ss,"%d",words[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                printf("%d, ",words[i]);
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
                                printf("%d\n",indices[i]);
                                sprintf(ss,"%d",indices[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                printf("%d, ",indices[i]);
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
                        printf("%d\n",words[cnt2-1]);
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
                        printf("%d\n",words[cnt2-1]);
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
                        printf("%d\n",indices[cnt2-1]);
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
                        printf("%d\n",indices[cnt2-1]);
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
            printf("-1\n");
            arm[acnt]='-', arm[acnt+1]='1' ,arm[acnt+2]='\n';
            acnt+=3;
            return;
        }
        if(count){
            printf("%d\n",cnt);
            char ss[100];
            sprintf(ss,"%d",cnt);
            strcat(arm,ss);
            acnt+=strlen(ss);
            arm[acnt]='\n';
            acnt++;
            return;
        }
        else{
            if(all){
                if(byword){
                    char ss[100];
                    for(int i=0;i<cnt-1;i++){
                        printf("%d, ",words[i]);
                        sprintf(ss,"%d",words[i]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]=',' , arm[acnt+1]=' ';
                        acnt+=2;
                    }
                    printf("%d\n",words[cnt-1]);
                    sprintf(ss,"%d",words[cnt-1]);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    arm[acnt]='\n';
                    acnt++;
                }
                else{
                    char ss[100];
                    for(int i=0;i<cnt-1;i++){
                        printf("%d, ",indices[i]);
                        sprintf(ss,"%d",indices[i]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]=',' , arm[acnt+1]=' ';
                        acnt+=2;
                    }
                    printf("%d\n",indices[cnt-1]);
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
                        printf("%d\n",words[at-1]);
                        sprintf(ss,"%d",words[at-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        printf("%d\n",words[0]);
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
                        printf("%d\n",indices[at-1]);
                        sprintf(ss,"%d",indices[at-1]);
                        strcat(arm,ss);
                        acnt+=strlen(ss);
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        printf("%d\n",indices[0]);
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
            printf("%d\n",cnt2);
            sprintf(ss,"%d",cnt2);
            strcat(arm,ss);
            acnt+=strlen(ss);
            arm[acnt]='\n';
            acnt++;
            return;
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
                                printf("%d\n",words[i]);
                                sprintf(ss,"%d",(words[i]));
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                printf("%d, ",words[i]);
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
                                printf("%d\n",indices[i]);
                                sprintf(ss,"%d",indices[i]);
                                strcat(arm,ss);
                                acnt+=strlen(ss);
                                arm[acnt]='\n';
                                acnt++;
                            }
                            else{
                                printf("%d, ",indices[i]);
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
                        printf("%d\n",words[cnt2-1]);
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
                        printf("%d\n",words[cnt2-1]);
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
                        printf("%d\n",indices[cnt2-1]);
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
                        printf("%d\n",indices[cnt2-1]);
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
    cnt=0;
    for(int i=0;i<1000;i++) indices[i]=0 ,words[i]=0, stars[i][0]=0 ,stars[i][1]=0 , stars[i][2]=0;
    fclose(fp);
}
void findNoprint(char text[],char directory[],int star,int all,int byword,int count,int at){
    int len = strlen(text)+1;
    FILE *fp;
    fp = fopen(directory,"r+");
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
        printf("File doesn't exist!\n");
        return;
    }
    if(all && at){
        printf("Invalid!\n");
        return;
    }
    findNoprint(text1,directory,star,0,0,0,0);
    int counter = cnt;
    int counter2=0;
    if(!counter){
        printf("-1\n");
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
int cntG;
void grep(char text[],char directory[],char option){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        printf("%s ",directory);
        strcat(arm,directory);
        acnt+=strlen(directory);
        arm[acnt]=' ';
        acnt++;
        printf("File doesn't exist!\n");
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
            printf("%s\n",directory);
            strcat(arm,directory);
            acnt+=strlen(directory);
            arm[acnt]='\n';
            acnt++;
        }
    }
    else if(option == 'a'){
        while(fgets(str,sizeof(str),fp)!=NULL){
            if(strstr(str,text) != NULL){
                printf("%s: %s",directory,str);
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
void undo(char directory[]){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        printf("File doesn't exist!\n");
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
    buf = fopen("buf.tmp","w+");
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
    remove("buf.tmp");
}
void autoIndent(char directory[]){
    FILE *fp;
    fp = fopen(directory,"r+");
    if(!fp){
        printf("File doesn't exist!\n");
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
void compare(char file1[],char file2[]){
    FILE *fp1;
    fp1 = fopen(file1,"r");
    if(!fp1){
        printf("File 1 doesn't exist!\n");
        return;
    }
    FILE *fp2;
    fp2 = fopen(file2,"r");
    if(!fp2){
        printf("File 2 doesn't exist!\n");
        return;
    }
    char str1[1000];
    char str2[1000];
    char word1[100][100];
    char word2[100][100];
    int dif=0;
    int lineNo=1,counter1=0,counter2=0,counter=0;
    if(feof(fp1)){
        while(fgets(str2,sizeof(str2),fp2)!=NULL){
            counter++;
        }
        rewind(fp2);
        printf(">>>>>>>>> #1 - #%d >>>>>>>>>\n",counter);
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
            printf("%s",str2);
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
            printf("<<<<<<<<< #%d - #%d <<<<<<<<<\n",lineNo1,lineNo-1);
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
                printf("%s",str1);
                strcat(arm,str1);
                acnt+=strlen(str1);
            }
            printf("\n");
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
                    printf("===== #%d =====\n",lineNo);
                    strcat(arm,"===== #");
                    acnt+=strlen("===== #");
                    char ss[100];
                    sprintf(ss,"%d",lineNo);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    strcat(arm," =====\n");
                    acnt+=strlen(" =====\n");
                    printf("%s\n",str1);
                    strcat(arm,str1);
                    acnt+=strlen(str1);
                    arm[acnt]='\n';
                    acnt++;
                    printf("%s\n",str2);
                    strcat(arm,str2);
                    acnt+=strlen(str2);
                    arm[acnt]='\n';
                    acnt++;
                }
                else{
                    printf("===== #%d =====\n",lineNo);
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
                            printf("%s ",word1[i]);
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        printf("\n");
                        arm[acnt]='\n';
                        acnt++;
                        for(int i=0;i<counter1;i++){
                            printf("%s ",word2[i]);
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        printf(">>%s<<",word2[counter2-1]);
                        strcat(arm,">>");
                        acnt+=strlen(">>");
                        strcat(arm,word2[counter2-1]);
                        acnt+=strlen(word2[counter2-1]);
                        strcat(arm,"<<");
                        acnt+=strlen("<<");
                        printf("\n");
                        arm[acnt]='\n';
                        acnt++;
                    }
                    else{
                        for(int i=0;i<counter2;i++){
                            printf("%s ",word1[i]);
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        printf(">>%s<<\n",word1[counter1-1]);
                        strcat(arm,">>");
                        acnt+=strlen(">>");
                        strcat(arm,word1[counter1-1]);
                        acnt+=strlen(word1[counter1-1]);
                        strcat(arm,"<<");
                        acnt+=strlen("<<");
                        for(int i=0;i<counter2;i++){
                            printf("%s ",word2[i]);
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        printf("\n");
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
                    printf("===== #%d =====\n",lineNo);
                    strcat(arm,"===== #");
                    acnt+=strlen("===== #");
                    char ss[100];
                    sprintf(ss,"%d",lineNo);
                    strcat(arm,ss);
                    acnt+=strlen(ss);
                    strcat(arm," =====\n");
                    acnt+=strlen(" =====\n");
                    printf("%s\n",str1);
                    strcat(arm,str1);
                    acnt+=strlen(str1);
                    arm[acnt]='\n';
                    acnt++;
                    printf("%s\n",str2);
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
                    printf("===== #%d =====\n",lineNo);
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
                            printf("%s ",word1[i]);
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        else{
                            printf(">>%s<< ",word1[i]);
                            strcat(arm,">>");
                            acnt+=strlen(">>");
                            strcat(arm,word1[i]);
                            acnt+=strlen(word1[i]);
                            strcat(arm,"<< ");
                            acnt+=strlen("<< ");
                        }
                    }
                    printf("\n");
                    arm[acnt]='\n';
                    acnt++;
                    for(int i=0;i<counter1;i++){
                        if(i!=diff){
                            printf("%s ",word2[i]);
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            arm[acnt]=' ';
                            acnt++;
                        }
                        else{
                            printf(">>%s<< ",word2[i]);
                            strcat(arm,">>");
                            acnt+=strlen(">>");
                            strcat(arm,word2[i]);
                            acnt+=strlen(word2[i]);
                            strcat(arm,"<< ");
                            acnt+=strlen("<< ");
                        }
                    }
                    printf("\n");
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
        printf(">>>>>>>>> #%d - #%d >>>>>>>>>\n",lineNo1,lineNo-1);
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
            printf("%s",str2);
            strcat(arm,str2);
            acnt+=strlen(str2);
        }
        printf("\n");
        arm[acnt]='\n';
        acnt++;
    }
}
void dirTree(char directory[],int depth,int tab){
    if(depth<-1){
        printf("Invalid depth!!\n");
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
                        printf("%c",179), printf("  ");
                        arm[acnt]=179,arm[acnt+1]=' ',arm[acnt+2]=' ';
                        acnt+=3;
                    }
                    if(tab){
                        printf("%c%c%c",195,196,196);
                        arm[acnt]=195,arm[acnt+1]=196,arm[acnt+2]=196;
                        acnt+=3;
                    }
                    strcpy(path,directory);
                    strcat(path,"/");
                    strcat(path,dir->d_name);
                    printf("%s\n",dir->d_name);
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
                        printf("%c",179), printf("  ");
                        arm[acnt]=179,arm[acnt+1]=' ',arm[acnt+2]=' ';
                        acnt+=3;
                    }
                    if(tab){
                        printf("%c%c%c",195,196,196);
                        arm[acnt]=195,arm[acnt+1]=196,arm[acnt+2]=196;
                        acnt+=3;
                    }
                    strcpy(path,directory);
                    strcat(path,"/");
                    strcat(path,dir->d_name);
                    printf("%s\n",dir->d_name);
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
    char destination[] = "dar.txt";
    char directory[] = "dar.txt";
    char directory2[] = "maram.txt";
    char command[300];
    while(1){
        char enter='\0';
        fgets(command,300,stdin);
        command[strlen(command)-1]='\0';
        if(strstr(command,"createfile --file ")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            createFile(dir);
        }
        else if(strstr(command,"insertstr --file ")){
            char dir[200];
            char text[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--str");
            p+=6;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        text[i-1]='\0';
                        break;
                    }
                    text[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        text[i]='\0';
                        break;
                    }
                    text[i]=command[p-command+i];
                }
            }
            p=strstr(command,"---pos");
            p+=7;
            char maram[20];
            int line;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==':'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            line = atoi(maram);
            int index;
            p+=strlen(maram)+1;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]=='\0'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            index=atoi(maram);
            insert(text,dir,line,index);
        }
        else if(strstr(command,"cat --file ")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]=='\0'){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            cat(dir);
        }
        else if(strstr(command,"removestr --file ")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--pos");
            p+=6;
            char maram[20];
            int line;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==':'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            line = atoi(maram);
            int index;
            p+=strlen(maram)+1;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]=='\0'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            index=atoi(maram);
            p=strstr(command,"-size");
            p+=6;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==' '){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            int size = atoi(maram);
            char direction;
            direction = command[strlen(command)-1];
            removeFrom(dir,line,index,direction,size);
        }
        else if(strstr(command,"copystr --file ")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--pos");
            p+=6;
            char maram[20];
            int line;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==':'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            line = atoi(maram);
            int index;
            p+=strlen(maram)+1;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==' '){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            index=atoi(maram);
            p=strstr(command,"-size");
            p+=6;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==' '){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            int size = atoi(maram);
            char direction;
            direction = command[strlen(command)-1];
            copy(dir,line,index,direction,size);
        }
        else if(strstr(command,"cutstr --file ")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--pos");
            p+=6;
            char maram[20];
            int line;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==':'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            line = atoi(maram);
            int index;
            p+=strlen(maram)+1;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==' '){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            index=atoi(maram);
            p=strstr(command,"-size");
            p+=6;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==' '){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            int size = atoi(maram);
            char direction;
            direction = command[strlen(command)-1];
            cut(dir,line,index,direction,size);
        }
        else if(strstr(command,"pastestr --file ")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--pos");
            p+=6;
            char maram[20];
            int line;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==':'){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            line = atoi(maram);
            int index;
            p+=strlen(maram)+1;
            for(int i=0;i<strlen(command);i++){
                if(command[p-command+i]==' '){
                    maram[i]='\0';
                    break;
                }
                maram[i]=command[p-command+i];
            }
            index=atoi(maram);
            paste(dir,line,index);
        }
        else if(strstr(command,"find --str ")){
            char dir[200];
            char text[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--str");
            p+=6;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        text[i-1]='\0';
                        break;
                    }
                    text[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        text[i]='\0';
                        break;
                    }
                    text[i]=command[p-command+i];
                }
            }
            int count=0;
            int all=0;
            int at=0;
            int byword=0;
            if(strstr(command,"-count")) count=1;
            if(strstr(command,"all")) all=1;
            if(strstr(command,"byword")) byword=1;
            if(p=strstr(command,"-at")){
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
        else if(strstr(command,"replace --str1 ")){
            char dir[200];
            char text[200];
            char text2[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--str1");
            p+=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        text[i-1]='\0';
                        break;
                    }
                    text[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        text[i]='\0';
                        break;
                    }
                    text[i]=command[p-command+i];
                }
            }
            p=strstr(command,"--str2");
            p+=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        text2[i-1]='\0';
                        break;
                    }
                    text2[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        text2[i]='\0';
                        break;
                    }
                    text2[i]=command[p-command+i];
                }
            }
            int all=0;
            int at=0;
            if(strstr(command,"all")) all=1;
            if(p=strstr(command,"-at")){
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
            replace(text,text2,dir,star,all,at);
        }
        else if(strstr(command,"grep ")){
            cntG=0;
            char dir[200],text[200];
            char *p=strstr(command,"--str");
            p+=6;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        text[i-1]='\0';
                        break;
                    }
                    text[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]==' '){
                        text[i]='\0';
                        break;
                    }
                    text[i]=command[p-command+i];
                }
            }
            char op;
            if(strstr(command,"-l")) op='l';
            else if(strstr(command,"-c")) op='c';
            else op='a';
            p =strstr(command,"--files");
            p +=8;
            int q=0;
            while(command[p-command+q]!='\0'){
                if(command[p-command+q]=='"'){
                    for(int i=1;i<strlen(command);i++){
                        if(command[p-command+q+i]=='"' && command[p-command+q+i-1]!='\\'){
                            dir[i-1]='\0';
                            break;
                        }
                        dir[i-1]=command[p-command+q+i];
                    }
                    q+=strlen(dir)+2;
                }
                else{
                    for(int i=0;i<strlen(command);i++){
                        if(command[p-command+q+i]==' '){
                            dir[i]='\0';
                            break;
                        }
                        dir[i]=command[p-command+q+i];
                    }
                    q+=strlen(dir)+1;
                }
                grep(text,dir,op);
                if(op=='a') printf("\n");
                if(op=='l') cntG=0;
                if(command[p-command+q-1]=='\0') break;
            }
            if(op=='c') printf("%d\n",cntG);
        }
        else if(strstr(command,"undo --file")){
            char dir[200];
            char *p =strstr(command,"--file");
            p +=7;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]=='\0'){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            undo(dir);
        }
        else if(strstr(command,"auto-indent")){
            char dir[200];
            char *p=strstr(command,"auto-indent");
            p +=12;
            if(command[p-command]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+i]=='"' && command[p-command+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+i]=='\0'){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+i];
                }
            }
            autoIndent(dir);
        }
        else if(strstr(command,"compare")){
            char dir[200],dir2[200];
            char *p=strstr(command,"compare");
            p+=8;
            int q=0;
            if(command[p-command+q]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+q+i]=='"' && command[p-command+q+i-1]!='\\'){
                        dir[i-1]='\0';
                        break;
                    }
                    dir[i-1]=command[p-command+q+i];
                }
                q+=strlen(dir)+2;
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+q+i]==' '){
                        dir[i]='\0';
                        break;
                    }
                    dir[i]=command[p-command+q+i];
                }
                q+=strlen(dir)+1;
            }
            if(command[p-command+q]=='"'){
                for(int i=1;i<strlen(command);i++){
                    if(command[p-command+q+i]=='"' && command[p-command+q+i-1]!='\\'){
                        dir2[i-1]='\0';
                        break;
                    }
                    dir2[i-1]=command[p-command+q+i];
                }
            }
            else{
                for(int i=0;i<strlen(command);i++){
                    if(command[p-command+q+i]==' '){
                        dir2[i]='\0';
                        break;
                    }
                    dir2[i]=command[p-command+q+i];
                }
            }
            compare(dir,dir2);
        }
        else if(strstr(command,"tree ")){
            char dep[10];
            int counter=0,depth;
            for(int i=5;i<strlen(command);i++){
                if(command[5]=='-'){
                    if(command[6]==1) depth=-1;
                    else depth=-2;
                }
                dep[counter]=command[i];
                counter++;
            }
            dep[counter]='\0';
            depth = atoi(dep);
            dirTree(".",depth,0);
        }
        else{
            printf("Invalid command!!\n");
        }
    }
}
