with open("names.txt", "r", encoding="utf-8") as f:
    for i,n in enumerate(f.readlines()):
        temp = n.replace("\n", "")

        if temp.isspace() or temp == "":
            continue
        temp = temp.split(" ")
        print(f"TEAMMEMBER:{{{' '.join(temp[:-1])}}}\nMATRICULE:{{{temp[-1]}}}".encode("utf-8"))
