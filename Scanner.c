#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<stdlib.h>
#define IDEN 0
#define REWD 1
#define INTE 2
#define FLOT 3
#define CHAR 4
#define STR 5
#define OPER 6
#define SPEC 7
#define SC 8
#define MC 9
#define PREP 10

char token[10];
char sign[2];
int filePos;
int fileCurrentLength = 0;
long fileTotalLength;
int line = 1;
FILE* file;
FILE* oFile;
char c;
char c_last;
char typeString[11][5] = { "IDEN", "REWD", "INTE", "FLOT", "CHAR", "STR", "OPER", "SPEC", "SC", "MC", "PREP" };
int type = -1;
void nextCh() { if (c != EOF)		c = fgetc(file);};
void put2Ch() {	token[0] = c_last;	token[1] = c;	token[2] = '\0'; nextCh();};
void print() { printf("%-4d%-8s", line, typeString[type]); fprintf(oFile,"%-4d%-8s", line, typeString[type]);};
void printNumber() {
	int endPos;
	if (c == '\n')
		endPos = ftell(file) - 1;
	else
		endPos = ftell(file);
	fseek(file, filePos-1, SEEK_SET);
	//printf("%d", ftell(file));
	nextCh();
	for (int i = filePos; i < endPos; i++) {
		printf("%c", c);
		fprintf(oFile,"%c", c);
		nextCh();
	}
	printf("\n");
	fprintf(oFile,"\n");
	for (int i = 0; i < 2; i++)
		sign[i] = '\0';
}
int isOp();
int isSpec();
int isSc();
int isMc();
int isRewd();
int isIden();
int isInt();
int isFlot();
int validFlotE();
int main() {
	oFile = fopen("output.txt", "w");
	if (!file)
		printf("Fail to open output.txt\n");

	file = fopen("Sample.c", "r");
	if (!file)
		printf("Faile to open Sample.c\n");
	else {
		/*********************************************
		* 處理順序:
		* PREP -> char -> string -> operator(OPER,SPEC) -> number(INTE,FLOT) -> alphabets(REWD then IDEN) -> comment(SC,MC) -> else
		***********************************************/
		nextCh();
		while (c != EOF) {
			if (c == '#') {
				type = PREP;
				print();
				while (c != '\n' && c != EOF) {
					printf("%c", c);
					fprintf(oFile,"%c", c);
					nextCh();
				}
				printf("\n");
				fprintf(oFile,"\n");
			}
			//bug!!!!!!!!char
			else if (c == '\'') {
				type = CHAR;
				print();
				nextCh();
				while (c != '\'') {
					printf("%c", c);
					fprintf(oFile, "%c", c);
					nextCh();
				}
				printf("\n");
				fprintf(oFile,"\n");
				nextCh();
			}	//char
			//string
			else if (c == '\"') {	//str
				type = STR;
				print();
				nextCh();
				while (c != '\"') {
					printf("%c", c);
					fprintf(oFile,"%c", c);
					nextCh();
					if (c == '\n' || c == EOF) {
						printf(" %+25s", "ERROR:Missing operator \"");
						fprintf(oFile, " %+25s", "ERROR:Missing operator \"");
						line++;
						break;
					}
				}
				nextCh();
				printf("\n");
				fprintf(oFile,"\n");
			}
			//spec
			else if (isSpec(c)) {	//spec
				print();
				printf("%c\n", c);
				fprintf(oFile,"%c\n", c);
				nextCh();
			}
			//op
			else if (isOp(c)) {	//oper or 註解
				print();
				
				printf("%s\n", token);
				fprintf(oFile,"%s\n", token);
			}
			//numbers
			else if (c >= 48 && c <= 57) {	//digit
				//integer?float?
				isInt();
			}
			//lower alphabet (rewd)
			else if (c >= 98 && c <= 119) {	//rewd
				//rewd
				isRewd();
			}
			//iden
			else if (c == '_' || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)) {	//iden
				token[0] = '\0';
				isIden();
			}

			else if (c == '\n') {
				nextCh();
				line++;
			}
			else if (c == ' ' || c == '\t') {
				nextCh();
			}
			else {	//error
				//printf("%4s\n", "error");
				//fprintf(oFile,"%4s\n", "error");
			}
		}
	}
	fclose(file);
	fclose(oFile);
	return 0;
}

//記得處理type跟nextCh()

int isOp() {
	type = OPER;
	c_last = c;
	//bug!!!!!!!!!!!!
	
	switch (c_last) {
	case EOF:
	case '\n':
		return 0;
		break;
	case '+':
		nextCh();
		if (c == '+' || c == '=') {
			put2Ch();
		}
		else if (c == '.') {
			nextCh();
			if (c >= '0' && c <= '9') {
				fseek(file, ftell(file) - 2, SEEK_SET);
				nextCh();
				sign[0] = '+';
				sign[1] = '\0';
				isFlot();
				return 0;
			}
			else {
				fseek(file, ftell(file) - 3, SEEK_SET);
				nextCh();
				type = OPER;
				token[0] = c;
				token[1] = '\0';
				nextCh();
				return 1;
			}
				
			return 0;
		}
		else if (c >= 48 && c <= 57) {
			sign[0] = '+';
			sign[1] = '\0';
			isInt();
			return 0;
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		
		return 1;
		break;
	case'-':
		nextCh();
		if (c == '-' || c == '=' || c == '>') {
			put2Ch();
		}
		else if (c >= '0' && c <= '9') {
			sign[0] = '-';
			sign[1] = '\0';
			isInt();
			return 0;
		}
		else if (c == '.') {
			nextCh();
			if (c >= '0' && c <= '9') {
				filePos = ftell(file);
				fseek(file, ftell(file) - 2, SEEK_SET);
				nextCh();
				sign[0] = '-';
				sign[1] = '\0';
				isFlot();
			}
			else {
				fseek(file, ftell(file) - 3, SEEK_SET);
				nextCh();
				type = OPER;
				token[0] = c;
				token[1] = '\0';
				nextCh();
				return 1;
			}

			return 0;
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '*':
		nextCh();
		if (c == '=') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '/':
		nextCh();
		if (c == '=') {
			put2Ch();
		}
		else if (c == '*') {
			nextCh();
			isMc();
			return 0;
		}
		else if (c == '/') {
			nextCh();
			isSc();
			return 0;
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		//comment or multiply
		return 1;
		break;
	case '=':
		nextCh();
		if (c == '=') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '%':
		nextCh();
		if (c == '=') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '>':
		nextCh();
		if (c == '=' || c == '>') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '<':
		nextCh();
		if (c == '=' || c == '<') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '!':
		nextCh();
		if (c == '=') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '&':
		nextCh();
		if (c == '&') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '|':
		nextCh();
		if (c == '|') {
			put2Ch();
		}
		else {
			token[0] = c_last;
			token[1] = '\0';
		}
		return 1;
		break;
	case '.':
		nextCh();
		if (c >= '0' && c <= '9') {
			fseek(file, ftell(file) - 2, SEEK_SET);
			nextCh();
			isFlot();
		}
		else {
			token[0] = '.';
			token[1] = '\0';
			return 1;
		}
		break;
	case ',':
	case '[':
	case ']':
	case '^':
		token[0] = c;
		token[1] = '\0';
		nextCh();
		return 1;
		break;
	default:
		return 0;
		break;
	}

}
int isSpec() {
	type = SPEC;
	if (c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == '?' || c == ':') {
		return 1;
	}
	else
		return 0;
}
int isSc() {
	type = SC;
	print();
	while (c != '\n' && c != EOF) {
		printf("%c", c);
		fprintf(oFile,"%c", c);
		nextCh();
	}
	printf("\n");
	fprintf(oFile,"\n");
	return 1;
}
int isMc() {
	type = MC;
	printf("%d-", line);
	fprintf(oFile,"%d-", line);
	char c_last = c;
	while (!(c == '/' && c_last == '*')) {
		if (c == EOF) {
			printf("%d %s",line, "ERROR    missing */");
			fprintf(oFile,"%d%s",line, "ERROR    missing */");
			return 0;
		}
		else if(c == '\n') {
			line++;
		}
		c_last = c;
		nextCh();
	}
	printf("%-2d%-8s\n", line, typeString[type]);
	fprintf(oFile,"%-2d%-8s\n", line, typeString[type]);
	nextCh();
	return 1;
}
int isRewd() {
	type = REWD;
	int i = 0;
	while ((c >= 97 && c <= 122) && i < 10) {
		token[i] = c;
		nextCh();
		i++;
	}
	if (i > 9) {
		isIden();
		return 0;
	}
	token[i] = '\0';
	if (isRewdString()) {
		print();
		printf("%-8s\n", token);
		fprintf(oFile,"%-8s\n", token);
	}
	else {
		isIden();
	}
	return 0;
}
int isRewdString() {
	char rewdStr[][10] = { "if", "else", "while", "for", "do", "switch", "case", "default", "continue", "int", "long", "float", "double", "char", "break", "static", "extern", "auto", "register", "sizeof", "union", "struct", "short", "enum", "return", "goto", "const", "signed", "unsigned", "typedef", "void" };
	for (int i = 0; i < 31 ; i++) {
		if (!strcmp(token, rewdStr[i]))
			return 1;
	}
	return 0;
}
int isIden() {
	type = IDEN;
	print();
	if (token[0] != '\0') {
		printf("%s", token);
		fprintf(oFile,"%s", token);
	}
		
	while (c == '_' || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c <= 57)) {
		printf("%c", c);
		fprintf(oFile,"%c", c);
		nextCh();
	}
	printf("\n");
	fprintf(oFile,"\n");
	return 1;
}
int isInt() {
	type = INTE;
	//fseek
	filePos = ftell(file);

	while (c >= '0' && c <= '9' || c == '.') {
		if (c == '.') {
			fseek(file, filePos - 1, SEEK_SET);
			nextCh();
			isFlot();
			return 0;
		}
		nextCh();
	}
	print();
	if (sign[0] != '\0') {
		printf("%s", sign);
		fprintf(oFile,"%s", sign);
	}
		
	printNumber();
	return 1;
}
int isFlot(){	
	type = FLOT;
	filePos = ftell(file);
	while (c >= '0' && c <= '9' && c != '.')
		nextCh();
	//cast .
	nextCh();
	while (c >= '0' && c <= '9')
		nextCh();

	if (validFlotE()) {
		//cast e
		nextCh();
		if (c == '+' || c == '-') 
			nextCh();
		while (c >= 48 && c <= 57) {
				nextCh();
		}
	}
	

	print();
	if (sign[0] != '\0') {
		printf("%s", sign);
		fprintf(oFile,"%s", sign);
	}
	printNumber();

	return 0;
}

int validFlotE(){
	int tempPos = ftell(file);
	if (c == 'e' || c == 'E') {
		nextCh();
		if (c >= '0' && c <= '9') {
			fseek(file, tempPos - 1, SEEK_SET);
			nextCh();
			return 1;
		}
		else if (c == '+' || c == '-') {
			nextCh();
			if (c >= '0' && c <= '9') {
				fseek(file, tempPos - 1, SEEK_SET);
				nextCh();
				return 1;
			}
		}
	}

	fseek(file, tempPos - 1, SEEK_SET);
	nextCh();

	return 0;
}