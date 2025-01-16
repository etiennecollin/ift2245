length = 0
with open("names.txt", "r", encoding="utf-8") as f:
    for i,n in enumerate(f.readlines()):
        temp = n.replace("\n", "")

        if temp.isspace() or temp == "":
            continue
        length += 1
        temp = temp.split(" ")
        print(f"TEAMMEMBER:{{{' '.join(temp[:-1])}}}\nMATRICULE:{{{temp[-1]}}}".encode("utf-8"))

if length == 0:
    print("!!\n!! Vous n'avez pas écrit vos noms dans names.txt !!\n!!")