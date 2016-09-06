struct BinaryTreeNode
{
  int m_nValue;
  BinaryTreeNode* m_pleft;
  BinaryTreeNode* m_pright;
};
BinaryTreeNode* Construct (int* preorder, int* inorder, int length) < 
{
  if (preorder — NULL || inorder — NULL || "Length <= 0) return NULL; return ConstructCore (preorder, preorder + length - 1, 
  inorder, inorder + length - 1) ; } 
  BinaryTreeNode* ConstructCore ( 
  int* startPreorder, int* endPreorder, 
  int* startlnorder, int* endlnorder } { 
  // BuffifiRff?iJ6f^-'-MJ;?Ji'tgSf5E№ 
  int rootValue - startPreorder[0]; 
  BinaryTreeNode* root - new BinaryTreeNode 0 ; 
  root->m_nValue - rootValue; root->m_pLeft = root->m_pRight - NULL; 
  if (startPreorder -= endPreorder) { 
  if (startlnorder == endlnorder 
  && *startP reorder == *startlnorder) return root; 
  eT.se 
  throw std::exception ("Invalid input.") ; > 
  // CT^ieiswiESffig-s&qtt 
  int* rootlnorder - startlnorder; whiT.e (rootlnorder <= endlnorder && *rootInDrder != rootVaT.ue) 
  ++ rootlnor'der; 
  if (rootingrder == endlnorder && * rootlnorder 1= rootVaT.ue) 
  throw std::exception ("Invalid input.") ; 
  int leftLength = rootlnorder - startlnorder; 
  int* leftPreorderEnd = startPreorder + leftLength; 
  if (leftLength > 0) •c // tSS£?M root->m_pLeft = ConstructCore (startPreorder + 1, 
  T.eftPreorderEnd, startlnorder, rootlnorder - 1) ; } 
  if (leftLength < endPreorder - start? reorder) < 
  root->m_pRight - ConstructCore (leftPreorderEnd + 1, endPreorder, rootlnorder + l, endlnorder) ; > } return root;