#include <vector>
#include <usuffix.h>

//usuffix.cpp -file that implements most of usuffix UkkonenTree
//This will be included from usuffixnode* -file, which implements UkkonenTree::Node.

//not a very elegant structure, but makes possible to compile nicely

//Class inheritance is not used, because I don't want that class UkkonenTree
//should assume that Node might sometimes be implemented differently
//(it would make things more complicated)




//Calculate the first character of an edge
void UkkonenTree::calcFirstCharacter(Edge& e){
    e.firstCharacter=(e.l >= 0 ? str[e.l] : -1);
}


int UkkonenTree::childIndex(Reference& ref){
    int id=ref.getChildIndex();
    if (id==-99) return ref.setChildIndex(ref.getS()->findChildIndex(str[ref.getL()]));
    return id;
}
    
    

UkkonenTree::Node* UkkonenTree::addChild(Node* node, int l, int r, int index){
    //Create new node
    Node* to=new Node(this); 
    //add it as a child of the parent node
    Edge e(to, l, r);
    node->addEdge(e, index);
    //return the new node
    return to;
}





//Ukkonen, page 12
void UkkonenTree::update(Reference& ref) {
    //follow boundary route from ref to end point
    int i=ref.getR()+1; //reference is to the state before the new character, i is the index of the new character
    Node* oldr=NULL;
    Pair<bool, Node*> endr=testAndSplit(ref, str[i]);
    while (!endr.first){ //While not an end point of the boundary route

        //this state is not an end-point => new leaf needs to bee added
        addChild(endr.second, i, -1, -1); //add new state node
        
        //add suffix link from previous node (if there is one) to this new new node
        if (oldr) oldr->setSuffixlink(endr.second); 
        oldr=endr.second;
        //follow the suffixlink of the current node in reference and canonize
        ref.setS(ref.getS()->getSuffixlink());
        
        canonize(ref);
        
        endr=testAndSplit(ref, str[i]);
    }
    if (oldr) oldr->setSuffixlink(ref.getS());
}

//Ukkonen, page 12
Pair<bool, UkkonenTree::Node*> UkkonenTree::testAndSplit(Reference& ref, int chr) {
    if (ref.getL()<=ref.getR()){ //referenced state not explicit
        int child_index=childIndex(ref);
        int i=ref.getS()->findEdge(child_index).l + (ref.getR()-ref.getL());//i=index corresponding to ref.r in edges l-r range 
                                                             //(edges l is not necessarily same as ref.l)
        if (str[i+1]==chr){ //transition already exists, state is an end-state => return true
            return {true, ref.getS()};
        }else{ //transition does not exist, splitting needed
            Edge pEdge=ref.getS()->findEdge(child_index); //Get original edge
            Node* newNode=addChild(ref.getS(), ref.getL(), ref.getR(), child_index);  //Add new node, replaces the original edge
            pEdge.l+=(ref.getR()-ref.getL()+1); //fix left boundary of original edge (must be done before addEdge)
            calcFirstCharacter(pEdge);
            newNode->addEdge(pEdge, -1); //Add original edge to newly created node
            return {false, newNode};
        }
    }else{ //referenced state explicit, no splitting needed
        if (childIndex(ref)<0){ //transition does not exist, return false
            return {false, ref.getS()};
        }else{ //transition exists, return true
            return {true, ref.getS()};
        }
    }
}

//Ukkonen page 13
void UkkonenTree::canonize(Reference& ref){
    if (ref.getL()<=ref.getR()){ //Only non-explicit references may need canonization
        Edge edge=ref.getS()->findEdge(childIndex(ref)); //Get correct transition from ref.s
        while (edge.l<0  || (edge.r - edge.l <= ref.getR() - ref.getL()  && edge.r!=-1)) { //While not canonized = while edge from ref.s is too short
                                                                                     //if edge is from aux (edge->l<0) it can be proved to always be too short (but comparison would not work)
             // go to next node and update ref.l
            ref.setS(edge.targetNode);
            if (edge.l<0)  ref.setL(ref.getL()+1); //edge is from aux
            else           ref.setL(ref.getL()+edge.r - edge.l +1);
            
            if (ref.getR()<ref.getL()) break; //reference to explicit node   
            edge=ref.getS()->findEdge(childIndex(ref));
        }
    }
}


bool UkkonenTree::findCSNodes(Node* node, Bitset& parentState, Vector<int>& terminator_positions, Vector<Pair<Node*, Pair<int, int> > >& positions, int depth, int last_character){
    Bitset state(terminator_positions.size());
    bool k=0;
    for (node->init_iteration(); node->iteration_ended()==0; node->next_iter()) { //Funny way to iterate children, but it works, which is nice.
        Edge child=node->current();
        if (child.r==-1){ //child is a leaf
            for (int i=0; i<(int)terminator_positions.size(); ++i){ //Find the string to which this leaf belongs
                if (terminator_positions[i]>=child.l){
                    state.setBitOn(i); // 
                    break;
                }
            }
        }else{
            int n_depth=depth+child.r-child.l+1;
            int n_last_character=child.r;
            if (findCSNodes(child.targetNode, state, terminator_positions, positions, n_depth, n_last_character)){ 
                //This node has a child that is common to all strings
                //This node cannot be the deepest node.
                k=1;
            }
        }
    }
    if (k) return 1; //
    
    if (state.popcount()==terminator_positions.size()){
        //This node is common to all strings and has no children that are common to all of them
        positions.push_back({node, {depth, last_character}});
        return 1;
    }
    //This node is not common to all strings.
    parentState|=state; // Update the state of the paretn node
    return 0;
}
    
Pair<int, Vector<int> > UkkonenTree::findLongestCommonSubstrings(Vector<int> terminator_positions){
    Bitset state(terminator_positions.size());
    Vector<Pair<Node*, Pair<int, int> > > positions; //Pointer to node and Pair: <depth, last character index>
    findCSNodes(root, state, terminator_positions, positions, 0, -1);
    int max_depth=0; //max_depth is also the length of longest common substring
    for (int i=0; i<(int)positions.size(); ++i){
        if (positions[i].second.first > max_depth) max_depth=positions[i].second.first;
    }
    Vector<int> ans;
    if (max_depth){ //If there is longest common substring, find all possibilities
        for (int i=0; i<(int)positions.size(); ++i){
            if (positions[i].second.first == max_depth){
                ans.push_back(positions[i].second.second-positions[i].second.first+1);
            }
        }
    }
    return {max_depth, ans};
}




void UkkonenTree::push(int chr){
    str.push_back(chr);
    update(activePoint);
    activePoint.setR(activePoint.getR()+1); //new active point
    canonize(activePoint);
}

void UkkonenTree::push(Vector<int>& astr, int delta){
    for (int i=0; i<(int)astr.size(); ++i){
        push(astr[i]+delta);
    }
}

void UkkonenTree::push(std::string& astr, int delta){
    Vector<int> data(astr);
    push(data, delta);
}


UkkonenTree::Reference UkkonenTree::getChild(Reference ref, int c){
    // If there exist a state ref+c, return canonized reference to it. If fail, returned Reference.s=NULL
    if (ref.getL()<=ref.getR()){
        int i=ref.getS()->findEdge(ref.getS()->findChildIndex(str[ref.getL()])).l + (ref.getR()-ref.getL());//i=index corresponding to ref.r in edges l-r range 
        if (i+1>=(int)str.size() || str[i+1]!=c){ 
            ref.setS(NULL);
        }else{
            ref.setR(ref.getR()+1);
            canonize(ref);
        }
    }else{
        int id=ref.getS()->findChildIndex(c);
        if (id>=0){
            int b=ref.getS()->findEdge(id).l;
            ref.setL(b); ref.setR(b);
            canonize(ref);
        }else{
            ref.setS(NULL);
        }
    }
    return ref;
}



bool UkkonenTree::isSubstring(Vector<int>& astr, int delta){
    Reference state(root, 1, 0);
    for (int i=0; i<(int)astr.size(); ++i){
        state=getChild(state, astr[i]+delta);
        if (state.getS()==NULL) return 0;
    }
    return 1;
}


bool UkkonenTree::isSubstring(std::string& astr, int delta){
    Vector<int> data(astr);
    return isSubstring(data, delta);
}


UkkonenTree::UkkonenTree(){
    root=new Node(this);
    aux=new Node(this);
    Edge e(root, -1, -1);
    aux->addEdge(e, -1);
    root->setSuffixlink(aux); //Suffix link from root to auxiliary state
    activePoint=Reference(root, 0, -1); //initial active state is root
}









//For drawing
std::string UkkonenTree::dotFormatDFS(Node* node, std::map<Node*, std::string>& names, int delta){
    if (!names.count(node)) names[node]=std::to_string(names.size()-2);
    std::string ans="";
    
    for (node->init_iteration(); node->iteration_ended()==0; node->next_iter()) {
        Edge child=node->current();
        Node* target=child.targetNode;
        int l=child.l;
        int r=child.r;
        std::string label="";
        for (int j=l; j<=(r==-1?(int)str.size()-1:r); ++j) label.push_back(str[j]+delta);
        if (r==-1)label+="...";
        ans+=dotFormatDFS(target, names, delta);
        ans+="\t"+names[node]+" -> "+names[target]+" [label=\""+label+"\"];\n";
    }
    if (!names.count(node->suffixlink)) names[node->suffixlink]=std::to_string(names.size()-2);
    if (node->suffixlink){
        ans+="\t"+names[node]+" -> "+names[node->suffixlink]+" [style=dashed arrowhead=halfopen];\n";
    }
    return ans;
}



std::string UkkonenTree::getDotFormat(int delta){
    std::string result="digraph{\n\taux;\n\troot=aux;\n";
    dnames[aux]="aux";
    dnames[root]="root";
    result+=dotFormatDFS(root, dnames, delta);

    result+="\t"+dnames[activePoint.getS()]+" [label=\""+dnames[activePoint.getS()]+"* ("+std::to_string(activePoint.getL())+", "+std::to_string(activePoint.getR())+")\"];\n";
    result+="\t"+dnames[aux]+" -> "+dnames[root]+";\n}";
    return result;
}