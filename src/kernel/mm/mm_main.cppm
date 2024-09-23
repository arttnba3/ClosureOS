export module kernel.mm;
export import :layout;
export import :pages;
export import :types;

import kernel.lib;

export namespace mm {

class KMemCache;

class PagePool {
public:
    PagePool(void);
    ~PagePool();
private:
    struct page *pgdb;
    size_t page_nr;
};

class KMemCache {
public:
private:
};



};
