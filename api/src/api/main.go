package main

import (
	"encoding/json"
	"flag"
	// "fmt"
	"crypto/rand"
	"io/ioutil"
	"math/big"
	"net/http"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/rs/cors"
	"gopkg.in/yaml.v2"
)

var (
	mapsDir = flag.String("maps-dir", "maps", "Path to the directory where the maps are located")
	posDir  = flag.String("pos-dir", "pos", "Path to the directory where the positions are located (with a 'local' folder and a 'global' one within it)")
	test    = flag.Bool("test", false, "If set, returns random data at each call")
)

type Coords struct {
	X float64 `json:"x"`
	Y float64 `json:"y"`
}

func main() {
	flag.Parse()

	mux := http.NewServeMux()
	mux.Handle("/maps/", http.StripPrefix("/maps/", http.FileServer(http.Dir(*mapsDir))))

	mux.HandleFunc("/robots", func(w http.ResponseWriter, req *http.Request) {
		files := make([]string, 0)
		fi, err := ioutil.ReadDir(*mapsDir)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		for _, file := range fi {
			filename := file.Name()
			extension := filepath.Ext(file.Name())
			// if file.Name() != "globale.png" && extension == ".png" {
			if extension == ".png" {
				files = append(files, filename[0:len(filename)-len(extension)])
			}
		}

		resp := struct {
			Robots []string `json:"robots"`
		}{Robots: files}
		content, err := json.Marshal(resp)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.Write(content)
	})

	mux.HandleFunc("/pos/local/", func(w http.ResponseWriter, req *http.Request) {
		var pos struct {
			Origin   []float64 `yaml:"origin"`
			Position []float64 `yaml:"position,omitempty"`
		}

		robotName := strings.TrimPrefix(req.URL.Path, "/pos/local/")
		if robotName == "" {
			http.Error(w, "Robot name is empty", 400)
			return
		}

		if *test {
			n, err := rand.Int(rand.Reader, big.NewInt(int64(3)))
			if err != nil {
				http.Error(w, err.Error(), 500)
				return
			}

			if n.Int64() != 0 {
				robotName = robotName + "-" + strconv.Itoa(int(n.Int64()))
			}
		}

		content, err := ioutil.ReadFile(*posDir + "/local/" + robotName + ".yaml")
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		if err = yaml.Unmarshal(content, &pos); err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		if len(pos.Origin) == 0 {
			pos.Origin = []float64{0, 0}
		}

		if len(pos.Position) == 0 {
			pos.Position = []float64{0, 0}
		}

		resp := struct {
			Origin   Coords `json:"origin"`
			Position Coords `json:"position"`
		}{
			Origin: Coords{
				X: pos.Origin[0],
				Y: pos.Origin[1],
			},
			Position: Coords{
				X: pos.Position[0],
				Y: pos.Position[1],
			},
		}

		content, err = json.Marshal(resp)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.Write(content)
	})

	mux.HandleFunc("/pos/global/", func(w http.ResponseWriter, req *http.Request) {
		var pos struct {
			Origin   []float64 `yaml:"origin"`
			Position []float64 `yaml:"position,omitempty"`
		}

		robotName := strings.TrimPrefix(req.URL.Path, "/pos/global/")
		if robotName == "" {
			http.Error(w, "Robot name is empty", 400)
			return
		}

		if *test {
			n, err := rand.Int(rand.Reader, big.NewInt(int64(3)))
			if err != nil {
				http.Error(w, err.Error(), 500)
				return
			}

			if n.Int64() != 0 {
				robotName = robotName + "-" + strconv.Itoa(int(n.Int64()))
			}
		}

		content, err := ioutil.ReadFile(*posDir + "/global/" + robotName + ".yaml")
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		if err = yaml.Unmarshal(content, &pos); err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		if len(pos.Origin) == 0 {
			pos.Origin = []float64{0, 0}
		}

		if len(pos.Position) == 0 {
			pos.Position = []float64{0, 0}
		}

		resp := struct {
			Origin   Coords `json:"origin"`
			Position Coords `json:"position"`
		}{
			Origin: Coords{
				X: pos.Origin[0],
				Y: pos.Origin[1],
			},
			Position: Coords{
				X: pos.Position[0],
				Y: pos.Position[1],
			},
		}

		content, err = json.Marshal(resp)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.Write(content)
	})

	handler := cors.Default().Handler(mux)
	if err := http.ListenAndServe("0.0.0.0:9090", handler); err != nil {
		panic(err)
	}
}
