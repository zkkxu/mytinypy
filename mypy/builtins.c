ObjType tp_print(TP) {
    int n = 0;
    ObjType e;
    TP_LOOP(e)
        if (n) { printf(" "); }
        tp_echo(tp,e);
        n += 1;
    TP_END;
    printf("\n");
    return tp_None;
}

ObjType tp_bind(TP) {
    ObjType r = TP_TYPE(FUNCTYPE);
    ObjType self = TP_OBJ();
    return tp_fnc_new(tp,
        r.fnc.ftype|2,r.fnc.cfnc,r.fnc.info->code,
        self,r.fnc.info->globals);
}

ObjType tp_min(TP) {
    ObjType r = TP_OBJ();
    ObjType e;
    TP_LOOP(e)
        if (compare(tp,r,e) > 0) { r = e; }
    TP_END;
    return r;
}

ObjType tp_max(TP) {
    ObjType r = TP_OBJ();
    ObjType e;
    TP_LOOP(e)
        if (compare(tp,r,e) < 0) { r = e; }
    TP_END;
    return r;
}

ObjType tp_copy(TP) {
    ObjType r = TP_OBJ();
    int type = r.type;
    if (type == LISTTYPE) {
        return cp_list(tp,r);
    } else if (type == DICTTYPE) {
        return _tp_dict_copy(tp,r);
    }
    tp_raise(tp_None,tp_string("(tp_copy) TypeError: ?"));
}


ObjType tp_len_(TP) {
    ObjType e = TP_OBJ();
    return tp_len(tp,e);
}

ObjType tp_assert(TP) {
    int a = TP_NUM();
    if (a) { return tp_None; }
    tp_raise(tp_None,tp_string("(tp_assert) AssertionError"));
}

ObjType tp_range(TP) {
    int a,b,c,i;
    ObjType r = to_list(tp);
    switch (tp->params.list.val->len) {
        case 1: a = 0; b = TP_NUM(); c = 1; break;
        case 2:
        case 3: a = TP_NUM(); b = TP_NUM(); c = TP_DEFAULT(tp_number(1)).number.val; break;
        default: return r;
    }
    if (c != 0) {
        for (i=a; (c>0) ? i<b : i>b; i+=c) {
            append_list(tp,r.list.val,tp_number(i));
        }
    }
    return r;
}

ObjType tp_system(TP) {
    char s[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),s,TP_CSTR_LEN);
    int r = system(s);
    return tp_number(r);
}

ObjType tp_istype(TP) {
    ObjType v = TP_OBJ();
    ObjType t = TP_STR();
    if (compare(tp,t,tp_string("string")) == 0) { return tp_number(v.type == TP_STRING); }
    if (compare(tp,t,tp_string("list")) == 0) { return tp_number(v.type == LISTTYPE); }
    if (compare(tp,t,tp_string("dict")) == 0) { return tp_number(v.type == DICTTYPE); }
    if (compare(tp,t,tp_string("number")) == 0) { return tp_number(v.type == TP_NUMBER); }
    if (compare(tp,t,tp_string("fnc")) == 0) { return tp_number(v.type == FUNCTYPE && (v.fnc.ftype&2) == 0); }
    if (compare(tp,t,tp_string("method")) == 0) { return tp_number(v.type == FUNCTYPE && (v.fnc.ftype&2) != 0); }
    tp_raise(tp_None,tp_string("(is_type) TypeError: ?"));
}


ObjType tp_float(TP) {
    ObjType v = TP_OBJ();
    int ord = TP_DEFAULT(tp_number(0)).number.val;
    int type = v.type;
    if (type == TP_NUMBER) { return v; }
    if (type == TP_STRING && v.string.len < 32) {
        char s[32]; memset(s,0,v.string.len+1);
        memcpy(s,v.string.val,v.string.len);
        if (strchr(s,'.')) { return tp_number(atof(s)); }
        return(tp_number(strtol(s,0,ord)));
    }
    tp_raise(tp_None,tp_string("(tp_float) TypeError: ?"));
}


ObjType tp_save(TP) {
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    ObjType v = TP_OBJ();
    FILE *f;
    f = fopen(fname,"wb");
    if (!f) { tp_raise(tp_None,tp_string("(tp_save) IOError: ?")); }
    fwrite(v.string.val,v.string.len,1,f);
    fclose(f);
    return tp_None;
}

ObjType tp_load(TP) {
    FILE *f;
    long l;
    ObjType r;
    char *s;
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    struct stat stbuf;
    stat(fname, &stbuf);
    l = stbuf.st_size;
    f = fopen(fname,"rb");
    if (!f) {
        tp_raise(tp_None,tp_string("(tp_load) IOError: ?"));
    }
    r = to_string(tp,l);
    s = r.string.info->s;
    fread(s,1,l,f);
/*    if (rr !=l) { printf("hmmn: %d %d\n",rr,(int)l); }*/
    fclose(f);
    return tp_track(tp,r);
}


ObjType tp_fpack(TP) {
    tp_num v = TP_NUM();
    ObjType r = to_string(tp,sizeof(tp_num));
    *(tp_num*)r.string.val = v;
    return tp_track(tp,r);
}

ObjType tp_abs(TP) {
    return tp_number(fabs(tp_float(tp).number.val));
}
ObjType tp_int(TP) {
    return tp_number((long)tp_float(tp).number.val);
}
tp_num _roundf(tp_num v) {
    tp_num av = fabs(v); tp_num iv = (long)av;
    av = (av-iv < 0.5?iv:iv+1);
    return (v<0?-av:av);
}
ObjType tp_round(TP) {
    return tp_number(_roundf(tp_float(tp).number.val));
}

ObjType tp_exists(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    return tp_number(!stat(fname,&stbuf));
}

ObjType tp_mtime(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    if (!stat(fname,&stbuf)) { return tp_number(stbuf.st_mtime); }
    tp_raise(tp_None,tp_string("(tp_mtime) IOError: ?"));
}

int _tp_lookup_(TP,ObjType self, ObjType k, ObjType *meta, int depth) {
    int n = _tp_dict_find(tp,self.dict.val,k);
    if (n != -1) {
        *meta = self.dict.val->items[n].val;
        return 1;
    }
    depth--; if (!depth) { tp_raise(0,tp_string("(tp_lookup) RuntimeError: maximum lookup depth exceeded")); }
    if (self.dict.dtype && self.dict.val->meta.type == DICTTYPE && _tp_lookup_(tp,self.dict.val->meta,k,meta,depth)) {
        if (self.dict.dtype == 2 && meta->type == FUNCTYPE) {
            *meta = tp_fnc_new(tp,meta->fnc.ftype|2,
                meta->fnc.cfnc,meta->fnc.info->code,
                self,meta->fnc.info->globals);
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP,ObjType self, ObjType k, ObjType *meta) {
    return _tp_lookup_(tp,self,k,meta,8);
}

#define TP_META_BEGIN(self,name) \
    if (self.dict.dtype == 2) { \
        ObjType meta; if (_tp_lookup(tp,self,tp_string(name),&meta)) {

#define TP_META_END \
        } \
    }

ObjType tp_setmeta(TP) {
    ObjType self = TP_TYPE(DICTTYPE);
    ObjType meta = TP_TYPE(DICTTYPE);
    self.dict.val->meta = meta;
    return tp_None;
}

ObjType tp_getmeta(TP) {
    ObjType self = TP_TYPE(DICTTYPE);
    return self.dict.val->meta;
}

ObjType tp_object(TP) {
    ObjType self = tp_dict(tp);
    self.dict.dtype = 2;
    return self;
}

ObjType tp_object_new(TP) {
    ObjType klass = TP_TYPE(DICTTYPE);
    ObjType self = tp_object(tp);
    self.dict.val->meta = klass;
    TP_META_BEGIN(self,"__init__");
        tp_call(tp,meta,tp->params);
    TP_META_END;
    return self;
}

ObjType tp_object_call(TP) {
    ObjType self;
    if (tp->params.list.val->len) {
        self = TP_TYPE(DICTTYPE);
        self.dict.dtype = 2;
    } else {
        self = tp_object(tp);
    }
    return self;
}


ObjType tp_getraw(TP) {
    ObjType self = TP_TYPE(DICTTYPE);
    self.dict.dtype = 0;
    return self;
}

ObjType tp_class(TP) {
    ObjType klass = tp_dict(tp);
    klass.dict.val->meta = tp_get(tp,tp->builtins,tp_string("object")); 
    return klass;
}

ObjType tp_builtins_bool(TP) {
    ObjType v = TP_OBJ();
    return (tp_number(tp_bool(tp, v)));
}
