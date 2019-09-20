// A C program to implement Ukkonen's Suffix Tree Construction 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#define MAX_CHAR 260

struct SuffixTreeNode {
	struct SuffixTreeNode *children[MAX_CHAR];

	//pointer to other node via suffix link 
	struct SuffixTreeNode *suffixLink;

	/*(start, end) interval specifies the edge, by which the
	node is connected to its parent node. Each edge will
	connect two nodes, one parent and one child, and
	(start, end) interval of a given edge will be stored
	in the child node. Lets say there are two nods A and B
	connected by an edge with indices (5, 8) then this
	indices (5, 8) will be stored in node B. */
	int start;
	int *end;

	/*for leaf nodes, it stores the index of suffix for
	the path from root to leaf*/
	int suffixIndex;
};

typedef struct SuffixTreeNode Node;

int text[10000]; //Input string 
Node *root = NULL; //Pointer to root node 

/*lastNewNode will point to newly created internal node,
waiting for it's suffix link to be set, which might get
a new suffix link (other than root) in next extension of
same phase. lastNewNode will be set to NULL when last
newly created internal node (if there is any) got it's
suffix link reset to new internal node created in next
extension of same phase. */
Node *lastNewNode = NULL;
Node *activeNode = NULL;

/*activeEdge is represeted as input string character
index (not the character itself)*/
int activeEdge = -1;
int activeLength = 0;

// remainingSuffixCount tells how many suffixes yet to 
// be added in tree 
int remainingSuffixCount = 0;
int leafEnd = -1;
int *rootEnd = NULL;
int *splitEnd = NULL;
int size = -1; //Length of input string 
int size1 = 0; //Size of 1st string

Node *newNode(int start, int *end)
{
	Node *node = (Node*)malloc(sizeof(Node));
	int i;
	for (i = 0; i < MAX_CHAR; i++)
		node->children[i] = NULL;

	/*For root node, suffixLink will be set to NULL
	For internal nodes, suffixLink will be set to root
	by default in current extension and may change in
	next extension*/
	node->suffixLink = root;
	node->start = start;
	node->end = end;

	/*suffixIndex will be set to -1 by default and
	actual suffix index will be set later for leaves
	at the end of all phases*/
	node->suffixIndex = -1;
	return node;
}

int edgeLength(Node *n) {
	return *(n->end) - (n->start) + 1;
}

int walkDown(Node *currNode)
{
	/*activePoint change for walk down (APCFWD) using
	Skip/Count Trick (Trick 1). If activeLength is greater
	than current edge length, set next internal node as
	activeNode and adjust activeEdge and activeLength
	accordingly to represent same activePoint*/
	if (activeLength >= edgeLength(currNode))
	{
		activeEdge += edgeLength(currNode);
		activeLength -= edgeLength(currNode);
		activeNode = currNode;
		return 1;
	}
	return 0;
}

void extendSuffixTree(int pos)
{
	/*Extension Rule 1, this takes care of extending all
	leaves created so far in tree*/
	leafEnd = pos;

	/*Increment remainingSuffixCount indicating that a
	new suffix added to the list of suffixes yet to be
	added in tree*/
	remainingSuffixCount++;

	/*set lastNewNode to NULL while starting a new phase,
	indicating there is no internal node waiting for
	it's suffix link reset in current phase*/
	lastNewNode = NULL;

	//Add all suffixes (yet to be added) one by one in tree 
	while (remainingSuffixCount > 0) {

		if (activeLength == 0)
			activeEdge = pos; //APCFALZ 

		// There is no outgoing edge starting with 
		// activeEdge from activeNode

		//printf("zzz int(text[activeEdge]) %i %c \n", int(text[activeEdge]), text[activeEdge]);

		if (activeNode->children[text[activeEdge]] == NULL)
		{
			//Extension Rule 2 (A new leaf edge gets created) 
			activeNode->children[text[activeEdge]] =
				newNode(pos, &leafEnd);

			/*A new leaf edge is created in above line starting
			from an existng node (the current activeNode), and
			if there is any internal node waiting for it's suffix
			link get reset, point the suffix link from that last
			internal node to current activeNode. Then set lastNewNode
			to NULL indicating no more node waiting for suffix link
			reset.*/
			if (lastNewNode != NULL)           //zzz 我觉得这段没用
			{
				lastNewNode->suffixLink = activeNode;
				lastNewNode = NULL;
			}
		}
		// There is an outgoing edge starting with activeEdge 
		// from activeNode 
		else
		{
			// Get the next node at the end of edge starting 
			// with activeEdge 
			Node *next = activeNode->children[text[activeEdge]];
			if (walkDown(next))//Do walkdown     //zzz 如果wd了，返回值为1，就会continue，
				//再走一遍循环（因为remainingSuffixCount值没减少，
				//所以不会漏掉一个extension）
			{
				//Start from next node (the new activeNode) 
				continue;
			}
			/*Extension Rule 3 (current character being processed
			is already on the edge)*/
			if (text[next->start + activeLength] == text[pos])   //zzz 这里next应该是activeNode吧?
				//zzz 这里就应该是next，注意上面if循环中有个continue
				//一旦walkdown会更新activeNode然后重走循环会更新next
				//为新的activeNode的child 
			{
				//If a newly created node waiting for it's 
				//suffix link to be set, then set suffix link 
				//of that waiting node to current active node 
				if (lastNewNode != NULL && activeNode != root)
				{
					lastNewNode->suffixLink = activeNode;
					lastNewNode = NULL;
				}

				//APCFER3 
				activeLength++;
				/*STOP all further processing in this phase
				and move on to next phase*/
				break;
			}

			/*We will be here when activePoint is in middle of
			the edge being traversed and current character
			being processed is not on the edge (we fall off
			the tree). In this case, we add a new internal node
			and a new leaf edge going out of that new node. This
			is Extension Rule 2, where a new leaf edge and a new
			internal node get created*/
			splitEnd = (int*)malloc(sizeof(int));
			*splitEnd = next->start + activeLength - 1;

			//New internal node 
			Node *split = newNode(next->start, splitEnd);
			activeNode->children[text[activeEdge]] = split;

			//New leaf coming out of new internal node 
			split->children[int(text[pos])] = newNode(pos, &leafEnd);  //zzz 不确定
			next->start += activeLength;
			split->children[int(text[next->start])] = next;    //zzz   不确定

			/*We got a new internal node here. If there is any
			internal node created in last extensions of same
			phase which is still waiting for it's suffix link
			reset, do it now.*/
			if (lastNewNode != NULL)
			{
				/*suffixLink of lastNewNode points to current newly
				created internal node*/
				lastNewNode->suffixLink = split;
			}

			/*Make the current newly created internal node waiting
			for it's suffix link reset (which is pointing to root
			at present). If we come across any other internal node
			(existing or newly created) in next extension of same
			phase, when a new leaf edge gets added (i.e. when
			Extension Rule 2 applies is any of the next extension
			of same phase) at that point, suffixLink of this node
			will point to that internal node.*/
			lastNewNode = split;
		}

		/* One suffix got added in tree, decrement the count of
		suffixes yet to be added.*/
		remainingSuffixCount--;
		if (activeNode == root && activeLength > 0) //APCFER2C1 
		{
			activeLength--;
			activeEdge = pos - remainingSuffixCount + 1;
		}
		else if (activeNode != root) //APCFER2C2 
		{
			activeNode = activeNode->suffixLink;
		}
	}
}

ofstream fout_tree("tree.txt");


void print(int i, int j)
{
	int k;
	for (k = i; k <= j; k++)
		fout_tree<<text[k]<<"-";
}

//Print the suffix tree as well along with setting suffix index 
//So tree will be printed in DFS manner 
//Each edge along with it's suffix index will be printed 
void setSuffixIndexByDFS(Node *n, int labelHeight, string attach)   //zzz添加一个attach 用来显示树的层级
{
	if (n == NULL) return;

	if (n->start != -1) //A non-root node 
	{
		//Print the label on edge from parent to current node 
		fout_tree << attach.c_str();
		print(n->start, *(n->end));
	}
	int leaf = 1;
	int i;
	for (i = 0; i < MAX_CHAR; i++)
	{
		if (n->children[i] != NULL)
		{
			if (leaf == 1 && n->start != -1)
				fout_tree<<"["<<n->suffixIndex<<"]"<<endl;

			//Current node is not a leaf as it has outgoing 
			//edges from it. 
			leaf = 0;
			setSuffixIndexByDFS(n->children[i], labelHeight +
				edgeLength(n->children[i]), string(attach + "==="));
		}
	}
	if (leaf == 1)
	{
		n->suffixIndex = size - labelHeight;
		//printf(" [%d]\n", n->suffixIndex);
		fout_tree << "[" << n->suffixIndex << "]" << endl;
	}
}

void freeSuffixTreeByPostOrder(Node *n)
{
	if (n == NULL)
		return;
	int i;
	for (i = 0; i < MAX_CHAR; i++)
	{
		if (n->children[i] != NULL)
		{
			freeSuffixTreeByPostOrder(n->children[i]);
		}
	}
	if (n->suffixIndex == -1)
		free(n->end);
	free(n);
}

/*Build the suffix tree and print the edge labels along with
suffixIndex. suffixIndex for leaf edges will be >= 0 and
for non-leaf edges will be -1*/
void buildSuffixTree()
{
	int i;
	rootEnd = (int*)malloc(sizeof(int));
	*rootEnd = -1;

	/*Root is a special node with start and end indices as -1,
	as it has no parent from where an edge comes to root*/
	root = newNode(-1, rootEnd);

	activeNode = root; //First activeNode will be root 
	for (i = 0; i < size; i++){
		extendSuffixTree(i);
		//int labelHeight = 0;									//zzz 这3句配合将每个phase的树都输出了
		//printf("\n ---------------------\n phase %d \n", i);        
		//setSuffixIndexByDFS(root, labelHeight, "");
	}
	int labelHeight = 0;
	setSuffixIndexByDFS(root, labelHeight, "");
	//Free the dynamically allocated memory 
	//freeSuffixTreeByPostOrder(root);       //在本cpp中与suffixTree.cpp不同,free工作放在后面int main里
}

int doTraversal(Node *n, int labelHeight, int* maxHeight,
	int* substringStartIndex)
{
	//cout << "  dt" << labelHeight;
	if (n == NULL)
	{
		return -1;
	}
	int i = 0;
	int ret = -1;
	if (n->suffixIndex < 0) //If it is internal node 
	{
		for (i = 0; i < MAX_CHAR; i++)
		{
			if (n->children[i] != NULL)
			{
				ret = doTraversal(n->children[i], labelHeight +
					edgeLength(n->children[i]),
					maxHeight, substringStartIndex);

				if (n->suffixIndex == -1)
					n->suffixIndex = ret;
				else if ((n->suffixIndex == -2 && ret == -3) ||
					(n->suffixIndex == -3 && ret == -2) ||
					n->suffixIndex == -4)
				{
					n->suffixIndex = -4;//Mark node as XY 
					//Keep track of deepest node 
				//	cout << "  lH:" << labelHeight;
					if (*maxHeight < labelHeight)
					{
						*maxHeight = labelHeight;
						*substringStartIndex = *(n->end) -
							labelHeight + 1;
					}
				}
			}
		}
	}
	else{
		//cout << " else:" << n->suffixIndex<<"  size1:"<<size1<<endl ;
		if (n->suffixIndex > -1 && n->suffixIndex < size1){//suffix of X 
		//	cout << "return -2" << endl;
			return -2;//Mark node as X 
		}
		else if (n->suffixIndex >= size1){//suffix of Y 
		//	cout << "return -3" << endl;
			return -3;//Mark node as Y 
		}
	}
	return n->suffixIndex;
}

void getLongestCommonSubstring(ofstream* fout)
{
	int maxHeight = 0;
	int substringStartIndex = 0;
	doTraversal(root, 0, &maxHeight, &substringStartIndex);

	int k;
	for (k = 0; k < maxHeight; k++){
		printf("%d ", text[k + substringStartIndex]);
		*fout << text[k + substringStartIndex] << " ";
	}
	if (k == 0){
		printf("No common substring");
		*fout << "No common substring";
	}
	else{
		printf(", of length: %d", maxHeight);
		*fout << ", of length: "<<maxHeight;
	}
	printf("\n");
	*fout << endl;
}

void my_fout(int * s1 , int size1, char* fname){
	ofstream fout(fname);
	for (int i = 0; i < size1; i++){
		if (i % 8 == 0)
		if (i % 16 == 0)
			fout << endl;
		else fout << " ";
		fout << "s[" << i << "]=" << int(s1[i]) << " ";
	}
	fout << "over" << endl;
	fout.close();
	return;
}


// driver program to test above functions 
int main(int argc, char *argv[])
{
	// strcpy_s(text, "abc"); buildSuffixTree(); 
	// strcpy_s(text, "xabxac#"); buildSuffixTree(); 
	// strcpy_s(text, "xabxa"); buildSuffixTree(); 
	// strcpy_s(text, "xabxa$"); buildSuffixTree(); 
	//strcpy_s(text, "xabxa#babxba$"); buildSuffixTree();
	// strcpy_s(text, "geeksforgeeks$"); buildSuffixTree(); 
	// strcpy_s(text, "THIS IS A TEST TEXT$"); buildSuffixTree(); 
	// strcpy_s(text, "AABAACAADAABAAABAA$"); buildSuffixTree(); 
	string filename = "in1_2016.txt";
	ifstream fin(filename);

	if (!fin){
		cout << "fin error";
		system("pause");
		exit(1);
	}
	int a;
	int size0=0;
	while (fin >> a){
		if (a == 512)
			break;
		text[size1] = a;
		//cout << a << text[size0] <<size0<< endl;
		size1++;
	}
	size1++;
	size0 = size1;
	text[size1 - 1] = 256;
	fin.close();
	cout << "size1:" << size1 << endl;
	//my_fout(s1, size1, "out1.txt");
	fin.open("in2_2012r2.txt");
	while (fin >> a){
		if (a == 512)
			break;
		text[size0] = a;
		//cout << a << text[size0] << size0 << endl;
		size0++;
	}
	text[size0] = 257;
	fin.close();
	size0++;
	my_fout(text, size0, "s.txt");
	cout << "size0:" << size0 << endl;
	
	size = size0;
	//strcpy_s(text, "xabxac#abcabxabcd$");
	ofstream fout1("LCS.txt");
	buildSuffixTree();
	fout1 << "Longest Common Substring is: ";
	//fout1.close();
	printf("Longest Common Substring is: ");
	getLongestCommonSubstring(&fout1);
	fout1.close();
	////Free the dynamically allocated memory 
	freeSuffixTreeByPostOrder(root);
	cout << "over" << endl;
	fout_tree.close();

	//size1 = 10;
	//strcpy_s(text, "xabxaabxa#babxba$"); 
	//buildSuffixTree();
	//printf("Longest Common Substring in xabxaabxa and babxba is: ");
	//getLongestCommonSubstring();
	////Free the dynamically allocated memory 
	//freeSuffixTreeByPostOrder(root);

	//size1 = 14;
	//strcpy_s(text, "GeeksforGeeks#GeeksQuiz$"); 
	//buildSuffixTree();
	//printf("Longest Common Substring in GeeksforGeeks and GeeksQuiz is: ");
	//getLongestCommonSubstring();
	////Free the dynamically allocated memory 
	//freeSuffixTreeByPostOrder(root);

	//size1 = 26;
	//strcpy_s(text, "OldSite:GeeksforGeeks.org#NewSite:GeeksQuiz.com$");
	//buildSuffixTree();
	//printf("Longest Common Substring in OldSite:GeeksforGeeks.org");
	//printf(" and NewSite:GeeksQuiz.com is: ");
	//getLongestCommonSubstring();
	////Free the dynamically allocated memory 
	//freeSuffixTreeByPostOrder(root);

	//size1 = 6;
	//strcpy_s(text, "abcde#fghie$");
	//buildSuffixTree();
	//printf("Longest Common Substring in abcde and fghie is: ");
	//getLongestCommonSubstring();
	////Free the dynamically allocated memory 
	//freeSuffixTreeByPostOrder(root);

	//size1 = 6;
	//strcpy_s(text, "pqrst#uvwxyz$");
	//buildSuffixTree();
	//printf("Longest Common Substring in pqrst and uvwxyz is: ");
	//getLongestCommonSubstring();
	////Free the dynamically allocated memory 
	//freeSuffixTreeByPostOrder(root);

	system("pause");
	//return 0;
	exit(0);
}
