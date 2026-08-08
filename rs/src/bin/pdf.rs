use std::collections::btree_map::BTreeMap;
use std::path::PathBuf;
use std::sync::Arc;

use lopdf::encryption::crypt_filters::{Aes128CryptFilter, CryptFilter};
use lopdf::{
    Dictionary, Document as D, EncryptionState, EncryptionVersion, Object, ObjectId, Permissions, Stream, dictionary
};
use rs::CollectVec;

rs::cli! {
    rot { input: S, output: Option<String>, angle: i64 = 90, force: bool = false }  => {
        let mut doc = checkin(&input);
        if let Some(inner) = &output && let Ok(val) = inner.parse() {
            angle = val;
            output = None;
        }
        for (_, id) in doc.get_pages() {
            doc.get_object_mut(id).unwrap().as_dict_mut().unwrap().set("Rotate", angle);
        }
        checkout(doc, &input, output, format!("rot-{angle}"), force);
    }
    sel { input: S, "example: 1,3:6,9" pages: S, output: Option<String>, force: bool = false } => {
        let mut doc = checkin(&input);
        let keep = parse_selection(&pages, doc.get_pages().keys().len());
        let del = doc.get_pages().keys().filter(|n| !keep.contains(&(**n as usize))).copied().colvec();
        doc.delete_pages(del.as_slice());
        doc.prune_objects();
        checkout(doc, &input, output, format!("sel-{pages}"), force);
    }
    encrypt { input: S, psk: S, "defaults to user_psk" owner_psk: Option<String>, output: Option<String>, force: bool = false } => {
        let mut doc = checkin(&input);
        let owner_password: &str = owner_psk.as_ref().unwrap_or(&psk);
        let state = encryption_state(&psk, owner_password, &doc);
        doc.encrypt(&state).unwrap();
        checkout(doc, &input, output, "encrypted".to_owned(), force);
    }
    decrypt { input: S, psk: S, output: Option<String>, force: bool = false } => {
        let mut doc = checkin(&input);
        doc.decrypt(&psk).unwrap();
        checkout(doc, &input, output, "encrypted".to_owned(), force);
    }
    mg { inputs: Vec<S>, output: S, force: bool = false } =>
        checkout(merge(inputs.iter().map(|p|checkin(p)).colvec()), "", Some(output), S::new(), force)
    rule { input: S, output: Option<String>, force: bool = false } =>
        checkout(add_ruler(checkin(&input)), &input, output, "ruled".to_owned(), force)
    crop { input: String, output: Option<String>, x: u32 = 0, y: u32 = 0, t: u32 = 0, b: u32 = 0, force: bool = false } =>
        checkout(crop(checkin(&input), x, y, t, b), &input, output, "ruled".to_owned(), force)
    ;
}

fn parse_selection(selector: &str, max: usize) -> Vec<usize> {
    let mut keep = vec![];
    for elt in selector.split(',') {
        let vec = elt.split(":").colvec();
        if vec.len() == 1 {
            keep.push(elt.parse().unwrap());
            continue;
        } else if vec.len() > 2 {
            upanic!("Invalid filtering syntax")
        }
        let from = vec[0];
        let to = vec[1];
        let fe = from.is_empty();
        let te = to.is_empty();
        if fe && te {
            upanic!("Tried to select all pages")
        } else if fe {
            keep.extend(1..=to.parse().unwrap());
        } else if te {
            keep.extend(from.parse().unwrap()..=max);
        } else {
            keep.extend(from.parse::<usize>().unwrap()..=to.parse().unwrap());
        }
    }
    keep
}

fn merge(documents: Vec<D>) -> D {
    let mut out = D::new();
    let (mut max, mut pages) = (1, vec![]);
    for mut d in documents {
        d.renumber_objects_with(max);
        max = d.max_id + 1;
        pages.extend(d.get_pages().into_values());
        out.objects.extend(d.objects);
    }
    let (pages_id, cat_id) = ((max, 0), (max + 1, 0));
    for id in &pages {
        out.get_object_mut(*id).unwrap().as_dict_mut().unwrap().set("Parent", pages_id);
    }
    out.objects.insert(
        pages_id,
        dictionary! {
            "Type" => "Pages",
            "Kids" => Object::Array(pages.iter().map(|&i| i.into()).collect()),
            "Count" => pages.len() as i64,
        }
        .into(),
    );
    out.objects.insert(cat_id, dictionary! { "Type" => "Catalog", "Pages" => pages_id }.into());
    out.trailer.set("Root", cat_id);
    out.max_id = max + 1;
    out.prune_objects();
    out
}

fn encryption_state(user_password: &str, owner_password: &str, document: &D) -> EncryptionState {
    let mut crypt_filters: BTreeMap<Vec<u8>, Arc<dyn CryptFilter>> = BTreeMap::new();
    crypt_filters.insert(b"StdCF".to_vec(), Arc::new(Aes128CryptFilter));
    let version = EncryptionVersion::V4 {
        document,
        encrypt_metadata: true,
        crypt_filters,
        stream_filter: b"StdCF".to_vec(),
        string_filter: b"StdCF".to_vec(),
        owner_password,
        user_password,
        permissions: Permissions::default(),
    };
    EncryptionState::try_from(version).unwrap()
}

fn checkout(mut doc: D, input: &str, path: Option<String>, data: S, force: bool) {
    let default = || format!("{}-{data}.pdf", input.strip_suffix(".pdf").unwrap());
    let path = path.unwrap_or_else(default);
    if !force && PathBuf::from(&path).exists() {
        upanic!("Output path {path} already exists");
    }
    doc.save(path).unwrap();
}

fn checkin(path: &str) -> D {
    let p = PathBuf::from(path);
    if !p.exists() {
        upanic!("Input path {path} doesn't exists");
    }
    if !p.extension().is_some_and(|ext| ext == "pdf") {
        upanic!("Input {path} doesn't seem to be a pdf");
    }
    D::load(path).unwrap()
}

fn inherited(doc: &D, page_id: ObjectId, key: &[u8]) -> Option<Object> {
    let mut id = page_id;
    loop {
        let dict = doc.get_dictionary(id).ok()?;
        if let Ok(o) = dict.get(key) {
            return Some(match o {
                Object::Reference(r) => doc.get_object(*r).ok()?.clone(),
                o => o.clone(),
            });
        }
        id = dict.get(b"Parent").ok()?.as_reference().ok()?;
    }
}

fn box_of(doc: &D, page_id: ObjectId, key: &[u8]) -> Option<[f32; 4]> {
    let obj = inherited(doc, page_id, key)?;
    let arr = obj.as_array().ok()?;
    let mut v = [0f32; 4];
    for (i, o) in arr.iter().take(4).enumerate() {
        let o = match o {
            Object::Reference(r) => doc.get_object(*r).ok()?,
            o => o,
        };
        v[i] = o.as_float().ok()?;
    }
    Some(v)
}

fn visible_box(doc: &D, page_id: ObjectId) -> Option<[f32; 4]> {
    box_of(doc, page_id, b"CropBox").or_else(|| box_of(doc, page_id, b"MediaBox"))
}

fn add_ruler(mut doc: D) -> D {
    let font_id = doc.add_object(dictionary! {
        "Type" => "Font", "Subtype" => "Type1", "BaseFont" => "Helvetica",
    });
    let q_id = doc.add_object(Stream::new(Dictionary::new(), b"q\n".to_vec()));
    let big_q_id = doc.add_object(Stream::new(Dictionary::new(), b"Q\n".to_vec()));

    let page_ids: Vec<ObjectId> = doc.get_pages().values().copied().collect();
    for page_id in page_ids {
        let Some([x0, y0, x1, y1]) = visible_box(&doc, page_id) else { continue };

        let mut c = S::new();
        c.push_str("q\n");
        let mut y = (y0 / 10.0).ceil() as i32 * 10;
        while (y as f32) <= y1 {
            let major = y % 50 == 0;
            c.push_str(if major { "1 0 0 RG 0.6 w\n" } else { "0.6 0.6 1 RG 0.2 w\n" });
            c.push_str(&format!("{} {} m {} {} l S\n", x0, y, x1, y));
            if major {
                c.push_str(&format!(
                    "BT /FRul 7 Tf 1 0 0 rg {} {} Td ({}) Tj ET\n",
                    x0 + 2.0,
                    y + 1,
                    y
                ));
            }
            y += 10;
        }
        let mut x = (x0 / 10.0).ceil() as i32 * 10;
        while (x as f32) <= x1 {
            let major = x % 50 == 0;
            c.push_str(if major { "1 0 0 RG 0.6 w\n" } else { "0.6 0.6 1 RG 0.2 w\n" });
            c.push_str(&format!("{} {} m {} {} l S\n", x, y0, x, y1));
            if major {
                c.push_str(&format!(
                    "BT /FRul 7 Tf 1 0 0 rg {} {} Td ({}) Tj ET\n",
                    x + 1,
                    y0 + 2.,
                    x
                ));
            }
            x += 10;
        }
        c.push_str("Q\n");

        if doc.get_dictionary(page_id).unwrap().get(b"Resources").is_err()
            && let Some(r) = inherited(&doc, page_id, b"Resources")
        {
            doc.get_dictionary_mut(page_id).unwrap().set("Resources", r);
        }
        let mut res = match doc.get_dictionary(page_id).unwrap().get(b"Resources").unwrap() {
            Object::Reference(r) => doc.get_dictionary(*r).unwrap().clone(),
            o => o.as_dict().unwrap().clone(),
        };
        let mut fonts = match res.get(b"Font") {
            Ok(Object::Reference(r)) => doc.get_dictionary(*r).unwrap().clone(),
            Ok(o) => o.as_dict().unwrap().clone(),
            Err(_) => Dictionary::new(),
        };
        fonts.set("FRul", Object::Reference(font_id));
        res.set("Font", Object::Dictionary(fonts));
        doc.get_dictionary_mut(page_id).unwrap().set("Resources", Object::Dictionary(res));

        let ruler_id = doc.add_object(Stream::new(Dictionary::new(), c.into_bytes()));
        let mut contents = vec![Object::Reference(q_id)];
        contents.extend(doc.get_page_contents(page_id).into_iter().map(Object::Reference));
        contents.push(Object::Reference(big_q_id));
        contents.push(Object::Reference(ruler_id));
        doc.get_dictionary_mut(page_id).unwrap().set("Contents", Object::Array(contents));
    }
    doc
}

fn crop(mut doc: D, x: u32, y: u32, t: u32, b: u32) -> D {
    let page_ids: Vec<ObjectId> = doc.get_pages().values().copied().collect();
    for page_id in page_ids {
        let Some([x0, y0, x1, y1]) = visible_box(&doc, page_id) else { continue };
        let new_box = vec![
            Object::Real(x0 + x as f32),
            Object::Real(y0 + b as f32),
            Object::Real(x1 - y as f32),
            Object::Real(y1 - t as f32),
        ];
        let page = doc.get_dictionary_mut(page_id).unwrap();
        page.set("CropBox", Object::Array(new_box.clone()));
        page.set("MediaBox", Object::Array(new_box));
    }
    doc
}
