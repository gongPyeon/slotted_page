#include "page.hpp"
#include <iostream> 
#include <cstring> 

//202211394 편강
void put2byte(void *dest, uint16_t data){ 
	*(uint16_t*)dest = data;
}

uint16_t get2byte(void *dest){ 
	return *(uint16_t*)dest;
}

page::page(uint16_t type){
	hdr.set_num_data(0); 
	hdr.set_data_region_off(PAGE_SIZE-1-sizeof(page*)); 
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));
	hdr.set_page_type(type);
}

uint16_t page::get_type(){
	return hdr.get_page_type();
}

uint16_t page::get_record_size(void *record){
	uint16_t size = *(uint16_t *)record;
	return size;
}

char *page::get_key(void *record){
	char *key = (char *)((uint64_t)record+sizeof(uint16_t));
	return key;
}

uint64_t page::get_val(void *key){
	uint64_t val= *(uint64_t*)((uint64_t)key+(uint64_t)strlen((char*)key)+1);
	return val;
}

void page::set_leftmost_ptr(page *p){
	leftmost_ptr = p;
}

page *page::get_leftmost_ptr(){
	return leftmost_ptr;	
}

uint64_t page::find(char *key){
	// Please implement this function in project 2.

	uint32_t num_data = hdr.get_num_data();
	uint16_t off = 0;
	uint16_t record_size = 0;
	void* offset_array = hdr.get_offset_array();
	void* data_region = nullptr;
	char* stored_key = nullptr;
	char* pre_key = nullptr;
	uint64_t stored_val = 0;
	uint64_t child_addr = 0;

	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
	
		if (strcmp(stored_key, key) == 0) { 
			return stored_val; 
		}else if(strcmp(stored_key, key) > 0){ // 찾고자 하는 키보다 큰 키를 만날 경우
			if(get_type() == INTERNAL){ // INTERNAL
				if(pre_key==NULL){
					child_addr = (uint64_t)get_leftmost_ptr(); // 맨 앞에 들어가야할 경우
				}else{
					child_addr = get_val((void*)pre_key); // child 주소를 알아낸다
				}
				
				return child_addr;
			}
		}
		pre_key = get_key(data_region); // 이전 키 저장
	}

	if(strcmp(stored_key, key) < 0){ // 마지막에 들어가야할 경우
		if(this->get_type() == INTERNAL){
			child_addr = get_val((void*)stored_key); // child 주소를 알아낸다
			return child_addr;
		}
	}

	return 0;
}

bool page::insert(char *key, uint64_t val){
	// Please implement this function in project 2.

	uint16_t record_size = 2 + strlen(key) + 1 + sizeof(val); // record size 계산
	short record_off = (hdr.get_data_region_off()+1) - record_size; // (freelist의 마지막 + 1) - record size = offset 계산


	if (is_full(record_size) || record_off < 0) { // full인지 확인
		return false;	
	}

	uint16_t* record_s = (uint16_t*)((uint64_t)this + (uint64_t)record_off); // size 넣기
	*record_s = record_size;
	char* record_key = (char*)((uint64_t)this + (uint64_t)record_off + sizeof(uint16_t)); // key 넣기
	memcpy(record_key, key, strlen(key) + 1);
	uint64_t* record_val = (uint64_t*)((uint64_t)this + (uint64_t)record_off + sizeof(uint16_t) + strlen(key) + 1); // val 넣기
	*record_val = val;
	

	void* offset_array = hdr.get_offset_array();
	uint32_t metadata = hdr.get_num_data(); // data의 개수



	/*정렬하는 부분*/

	void *pre=nullptr; 
	uint16_t off=0; 
	void *data_region=nullptr;

	if(metadata == 0){ //처음 추가하는 것일때

		uint16_t* offset_ptr = (uint16_t*)((uint64_t)offset_array + metadata * sizeof(uint16_t));
		*offset_ptr = record_off; //그다음 넣을 수 있는 공간을 가리킨다

	}else{ // 처음 추가하는게 아닐때

		for(int i = metadata-1; i >= 0; i--){ //정렬된 순서이므로 큰쪽부터 확인한다

			off= *(uint16_t *)((uint64_t)offset_array+i*2);	
			data_region = (void *)((uint64_t)this+(uint64_t)off); 
			pre = get_key(data_region);
			
			if(strcmp((char*)pre, key) <= 0 || i==0) {// key값이 같거나 크기 또는 i=0일 때
				uint16_t* offset_ptr = nullptr;
				
				if(i == 0 && strcmp((char*)pre, key) > 0){// 맨 첫번째에 들어가야할 때

					for(int j = metadata-1; j>=i; j--){ // 자리를 확보하기 위해 한칸씩 밀기
						off= *(uint16_t *)((uint64_t)offset_array + j * sizeof(uint16_t));	// 0 to 1, 1 to 2
						uint16_t* offset_move = (uint16_t*)((uint64_t)offset_array + (j+1) * sizeof(uint16_t));
						*offset_move = off;
					}
					offset_ptr = (uint16_t*)((uint64_t)offset_array + i * sizeof(uint16_t)); // 0 자리 확보

				}else{ // 중간 또는 끝에 들어가야할 때

					for(int j = metadata-1; j>i; j--){ // i번째 바로 뒤에 들어가야하기 때문에 j>i로 범위 설정, 자리를 확보하기 위해 한칸씩 밀기
						off= *(uint16_t *)((uint64_t)offset_array + j * sizeof(uint16_t));	
						uint16_t* offset_move = (uint16_t*)((uint64_t)offset_array + (j+1) * sizeof(uint16_t));
						*offset_move = off;
					}

					offset_ptr = (uint16_t*)((uint64_t)offset_array + (i+1) * sizeof(uint16_t)); // i번째 바로 뒤
				}

				*offset_ptr = record_off; // record_offset (시작번지 저장)

				break;
			}
		}
	}	

	hdr.set_num_data(metadata + 1); 
	hdr.set_data_region_off(record_off-1); //region_off는 freelist 마지막 번지이므로 -1

	print();

	return true;
}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 3.

	// 새로운 노드에 절반의 엔트리를 복사
	page *new_page = new page(get_type()); 
	int num_data = hdr.get_num_data(); 
	void *offset_array=hdr.get_offset_array(); 
	void *stored_key=nullptr; 
	uint16_t off=0; 
	uint64_t stored_val=0; 
	void *data_region=nullptr;

	int medium = (num_data / 2);

	for(int i=medium; i<num_data; i++){ 
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off); 
		stored_key = get_key(data_region); 
		stored_val= get_val((void *)stored_key); 
		new_page->insert((char*)stored_key,stored_val); 
	}
	new_page->set_leftmost_ptr(get_leftmost_ptr()); 
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header))); //page + header
	
	// 기존 노드의 절반을 삭제
	defrag();

	// parent_key에 medium번째 key를 추가한다
	off = *(uint16_t *)((uint64_t)offset_array);
    data_region = (void *)((uint64_t)new_page + (uint64_t)off);
	*parent_key = get_key(data_region);
	printf("parentKey: %s\n", *parent_key);


	//print();
	// 새로 생긴 노드의 주소를 리턴
	return new_page;
}

bool page::is_full(uint64_t inserted_record_size) {
	// Please implement this function in project 2.

	uint64_t total_space = PAGE_SIZE; // 1부터 256까지의 방
	uint64_t record_size = PAGE_SIZE - 1 - sizeof(page*) - hdr.get_data_region_off();
	// leftmost - offset, region_off이 freelist 마지막 번지를 가리키고 있으므로 -1을 해줘야한다

	uint64_t used_space = sizeof(slot_header) + sizeof(page*) + record_size + (hdr.get_num_data() * 2);
	// page class 변수 : header + leftmost + recordsize + offset개수

	if ((total_space - used_space) < inserted_record_size + 2) // 남은 공간이 레코드 사이즈 + 2(offset 주소크기) 보다 작다면 full하다는 의미
		return true;

	return false; // 남은 공간이 레코드 사이즈 + 2(offset 주소)와 같거나 크다면 full하지 않다는 의미
}


void page::defrag(){
	page *new_page = new page(get_type()); 
	int num_data = hdr.get_num_data(); 
	void *offset_array=hdr.get_offset_array(); 
	void *stored_key=nullptr; 
	uint16_t off=0; 
	uint64_t stored_val=0; 
	void *data_region=nullptr; 

	for(int i=0; i<num_data/2; i++){ 
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off); 
		stored_key = get_key(data_region); 
		stored_val= get_val((void *)stored_key); 
		new_page->insert((char*)stored_key,stored_val); 
	}	
	new_page->set_leftmost_ptr(get_leftmost_ptr()); 

	memcpy(this, new_page, sizeof(page));
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header))); //page + header
	delete new_page;

}

void page::print(){
	uint32_t num_data = hdr.get_num_data();
	uint16_t off=0;
	uint16_t record_size= 0;
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint64_t stored_val=0;

	printf("## slot header\n");
	printf("Number of data :%d\n",num_data);
	printf("offset_array : |");
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);
		printf(" %d |",off);
	}
	printf("\n");

	void *data_region=nullptr;
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val= get_val((void *)stored_key);
		printf("==========================================================\n");
		printf("| data_sz:%u | key: %s | val :%lu | key_len:%lu\n",record_size,(char*)stored_key, stored_val,strlen((char*)stored_key));

	}
}




