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

    // insert할 페이지를 어떻게 찾지?
	while(current->get_type() != LEAF){

	}

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

uint64_t btree::lookup(char *key){ // 처음엔 b+트리에서 header를 어떻게 접근할지 고민했다 => find함수를 사용하면 된다는 것을 나중에 발견함
	// Please implement this function in project 3.

	page* current = root; // root 노드 설정

	uint64_t stored_val = current->find(key);
	if (stored_val == 0) { // key를 못찾았으면
		return 0;
	}else{
		return stored_val;
	}
}
