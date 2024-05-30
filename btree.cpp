#include "btree.hpp"
#include <iostream>
#define DEGREE 3 // 차수

btree::btree(){
	root = new page(LEAF);
	height = 1;
};

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 3.

	char *parent_key = NULL; // root의 key를 할당
	page *current = root;
    page *parent = nullptr;

    // insert할 페이지를 어떻게 찾지? => find함수에서 child addr를 리턴해주는 방식을 찾음
	while(current->get_type() != LEAF){
		parent = current;
		current = (page*)current->find(key);
	}

	if(sizeof(current) < DEGREE * 2 - 1){ // 50% 이하로 데이터가 채워져있을 땐 바로 삽입한다
		current->insert(key, val);

	}else{ // 50% 이상으로 데이터가 채워져있을 땐 split한다
		if(root->get_type() == LEAF){// root가 leaf이면 부모가 없다는 뜻 //root가 internal이면 밑에 자식이 있다는 뜻
			// root가 (잎노드) 가득차서 split이 발생한 경우
			page *new_root = new page(INTERNAL);
			new_root->set_leftmost_ptr(current);
			root = new_root;
			height++;

			parent = root;
		}

		
		page* new_child = current->split(key, val, &parent_key); // parent key가 필요한 이유 : medium
		parent->set_leftmost_ptr(new_child); // parent의 오른쪽이 새로 만들어진 페이지를 가리키도록한다, 왼 쪽 child는 이미 만들어졌음
		this->insert(parent_key, (uint64_t)new_child); // parent key와 자식 주소를 부모 노드에 저장하기 위해 insert를 부른다 (또 split이 일어날 수 있기 때문에)
		
	}

}

uint64_t btree::lookup(char *key){ // 처음엔 b+트리에서 header를 어떻게 접근할지 고민했다 => find함수를 사용하면 된다는 것을 나중에 발견함
	// Please implement this function in project 3.

	page* current = root; // root 노드 설정

	uint64_t stored_val = 0;
	
	while(current != NULL){
		current->find(key);
		if (stored_val == 0) { // key를 못찾았으면
			return 0;
		}else{
			if(current->get_type() == INTERNAL){
				current = (page*)stored_val; // child 주소를 알아낸다
			}else{
				return stored_val;
			}		
		}
	}

	return 0;
	
}
