#include "page.hpp"
#include <iostream> 
#include <cstring> 

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
	//printf("\n\nfind\n");
	uint32_t num_data = hdr.get_num_data();
	uint16_t off = 0;
	uint16_t record_size = 0;
	void* offset_array = hdr.get_offset_array();
	void* data_region = nullptr;
	char* stored_key = nullptr;
	uint64_t stored_val = 0;

	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		//printf("***key : %s\n", key);
		//printf("off : %d, record_size : %d, stored_key : %s, stored_val : %d\n",off, record_size, stored_key, stored_val);
		//printf("strcmp : %d\n",strcmp(stored_key, key));
		if (strcmp(stored_key, key) == 0) { 
			return stored_val; 
		}
	}
	//printf("\n");
	return 0;
}

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 2.

	//printf("\n\ninsert key: %s, valL %d\n", key, val);
	uint16_t record_size = 2 + strlen(key) + 1 + sizeof(val);
	//printf("strlen : %d, sizeof : %d, record_size : %lu\n", strlen(key), sizeof(val), record_size);

	short record_off = (hdr.get_data_region_off()+1) - record_size;
	//printf("record_off :%d %d\n", record_off, hdr.get_data_region_off()); 

	if (is_full(record_size) || record_off < 0) { 
		return false;	
	}

	//printf("hdr : %d this : %d\n",sizeof(slot_header), sizeof(this));
	uint16_t* record_s = (uint16_t*)((uint64_t)this + (uint64_t)record_off); 
	*record_s = record_size;
	//printf("record_s : %lu\n", *record_s);

	char* record_key = (char*)((uint64_t)this + (uint64_t)record_off + sizeof(uint16_t)); 
	memcpy(record_key, key, strlen(key) + 1);
	//printf("record_key : %s\n", record_key);
	//printf("record_key : %c\n", *record_key);
	
	// uint64_t* record_val = (uint64_t*)((uint64_t)key+(uint64_t)strlen((char*)key)+1);
	uint64_t* record_val = (uint64_t*)((uint64_t)this + (uint64_t)record_off + sizeof(uint16_t) + strlen(key) + 1); 
	*record_val = val;
	//printf("record_val : %lu\n\n", *record_val2);
	

	void* offset_array = hdr.get_offset_array();
	uint32_t metadata = hdr.get_num_data();

	/*정렬하는 부분*/
	void *pre=nullptr; 
	uint16_t off=0; 
	void *data_region=nullptr;

	if(metadata == 0){ //처음 추가하는 것일때
		uint16_t* offset_ptr = (uint16_t*)((uint64_t)offset_array + metadata * sizeof(uint16_t));
		*offset_ptr = record_off; //그다음 넣을 수 있는 공간을 가리킨다
	}else{ // 처음 추가하는게 아닐때
		for(int i = metadata-1; i >= 0; i--){ 
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off); 
		pre = get_key(data_region);
		//printf("pre : %s\n\n", (char*)pre);
		if(strcmp((char*)pre, key) <= 0 || i==0) {// key값이 같거나 크기 또는 i=0일 때
			uint16_t* offset_ptr = nullptr;
			//printf("strcmp: %d\n", strcmp((char*)pre, key));
			if(i == 0 && strcmp((char*)pre, key) > 0){// 맨 첫번째에 들어가야할 때
				for(int j = metadata-1; j>=i; j--){
					off= *(uint16_t *)((uint64_t)offset_array + j * sizeof(uint16_t));	
					uint16_t* offset_move = (uint16_t*)((uint64_t)offset_array + (j+1) * sizeof(uint16_t));
					*offset_move = off;
				}
				offset_ptr = (uint16_t*)((uint64_t)offset_array + i * sizeof(uint16_t));
			}else{ // 중간 또는 끝에 들어가야할 때
				for(int j = metadata-1; j>i; j--){
					off= *(uint16_t *)((uint64_t)offset_array + j * sizeof(uint16_t));	
					uint16_t* offset_move = (uint16_t*)((uint64_t)offset_array + (j+1) * sizeof(uint16_t));
					*offset_move = off;
				}
				offset_ptr = (uint16_t*)((uint64_t)offset_array + (i+1) * sizeof(uint16_t));
			}
			*offset_ptr = record_off;

			break;
		}
		}
	}	
	
	//printf("%d %d %d\n", (uint64_t)offset_array + metadata * sizeof(uint16_t), metadata * sizeof(uint16_t), *offset_ptr);
	//printf("size확인 : %d\n\n", *(uint16_t*)((uint64_t)this + (uint64_t)record_off));
	//size가 0이 나온다 -> g


	hdr.set_num_data(metadata + 1); 
	hdr.set_data_region_off(record_off-1);

	print();

	return true;
}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 3.
	page *new_page;
	return new_page;
}

bool page::is_full(uint64_t inserted_record_size) {
	// Please implement this function in project 2.

	uint64_t total_space = PAGE_SIZE; // 1부터 256까지의 방
	//printf("***\n");
	uint64_t record_size = PAGE_SIZE-1-sizeof(page*) - hdr.get_data_region_off();

	uint64_t used_space = sizeof(slot_header) + sizeof(page*) + record_size + (hdr.get_num_data() * 2);
	// header + page* + record + offset배열
	//printf("used space : %lu\n", record_size);
	//printf("***\n");

	if ((total_space - used_space) < inserted_record_size + 2)
		return true;

	return false;
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




