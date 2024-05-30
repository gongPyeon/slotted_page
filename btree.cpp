#include "btree.hpp"
#include <iostream>
#define DEGREE 3 // 차수

btree::btree(){
	root = new page(LEAF);
	height = 1;
};

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 3.

	char *parent_key; // root의 key를 할당
	page *current = root;
    page *parent = nullptr;

    // leaf로 내려간다 ?

	if(sizeof(parent_key) < DEGREE * 2 - 1){ // 50% 이하로 데이터가 채워져있을 땐 바로 삽입한다
		root->insert(key, val);

	}else{ // 50% 이상으로 데이터가 채워져있을 땐 split한다
		
		if(root->get_type() == INTERNAL){
			// leaf 또는 internal 노드가 가득차서 split이 발생한 경우
			root->insert(key, val); // insert 후 분할
			root->split(key, val, &parent_key); // parent key가 필요한 이유 : medium

		}else{
			// root가 (잎노드) 가득차서 split이 발생한 경우
			page *new_root = new page(INTERNAL);
			new_root->set_leftmost_ptr(current);
		
			page *new_page = root->split(key, val, &parent_key);
			new_root->insert(parent_key, (uint64_t)current);

			root = new_root;
			height++;
		}
		
	}

}

uint64_t btree::lookup(char *key){
	// Please implement this function in project 3.

	page* btree_node = root; // root 노드 설정
	page* child_addr = nullptr;

	while (btree_node != nullptr) { // null이면 더이상 leaf가 없는 것이기 때문
		
		uint64_t stored_val = btree_node->find(key);
			if (stored_val == 0) { // key를 못찾았으면
				return 0;
			}else{
				child_addr = (page*)stored_val;
			}
		}
}
